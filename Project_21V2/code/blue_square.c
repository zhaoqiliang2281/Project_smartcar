#include "blue_square.h"
#include "square_kalman_filter.h"

/*
 * The old Vision_dis_tx_nosun project did not measure the sign from the
 * bounding-box width.  It first selected one connected square component,
 * extracted its four corners, and then compared the two vertical edges.
 * The legacy file name is retained, but the detector now selects a filled
 * HSV-selected purple or blue circular component.
 */

#define BLUE_MARK_VISITED       (128u)
#define BLUE_SPAN_STACK_SIZE    (512u)
/* 尺寸门限（等价于距离门限）：目标直径 150mm，distance_mm = 10500/高度。
   跟踪距离约 0.1~2m 时，目标面积约几十到几百像素。
   面积上限从 20000 收紧到 10000，拒绝"大块蓝色物体"（蓝墙、别的车）；
   面积下限从 20 提到 40，拒绝过小的蓝色噪点。 */
#define BLUE_MIN_AREA           (40u)
#define BLUE_MAX_AREA           (10000u)
#define BLUE_MIN_SIZE           (5u)
#define BLUE_MIN_FILL_PERMILLE  (450u)
#define BLUE_MAX_FILL_PERMILLE  (950u)
#define BLUE_MIN_ASPECT_PERMILLE (650u)

/* 水平偏移量（像素），两条转向参考线偏离画面中心的位置，由 blue_square_draw() 用青色绘制。
   当目标中心越过其中一条参考线时，转向比例增益会提升，使车辆更猛地转向，以保持靠近画面边缘的
   目标（摄像头FOV窄至90度）不会离开图像。 */
#define STEER_REFERENCE_OFFSET_PX   (30u)

static uint8 blue_mask[SCC8660_H][SCC8660_W];

typedef struct
{
    uint16 y;
    uint16 x_left;
    uint16 x_right;
} blue_span_t;

typedef struct
{
    uint16 area;
    uint32 sum_x;
    uint32 sum_y;
    uint16 x_min;
    uint16 x_max;
    uint16 y_min;
    uint16 y_max;
    uint16 width;
    uint16 height;
    uint16 fill_permille;
    uint16 aspect_permille;
    uint16 seed_x;
    uint16 seed_y;
    uint8 touch_border;
    int16 min_sum;
    int16 max_sum;
    int16 min_diff;
    int16 max_diff;
    uint16 tl_x;
    uint16 tl_y;
    uint16 tr_x;
    uint16 tr_y;
    uint16 br_x;
    uint16 br_y;
    uint16 bl_x;
    uint16 bl_y;
} blue_component_t;

static blue_span_t blue_span_stack[BLUE_SPAN_STACK_SIZE];

/* Runtime colour thresholds.  Initialized to the compile-time defaults above,
   then updated at runtime by blue_square_sample_threshold() / set_threshold(). */
static color_threshold_t g_threshold =
{
#if (TARGET_CIRCLE_COLOR == CIRCLE_COLOR_BLUE)
    BLUE_H_MIN, BLUE_H_MAX, BLUE_S_MIN, BLUE_V_MIN
#else
    PURPLE_H_MIN, PURPLE_H_MAX, PURPLE_S_MIN, PURPLE_V_MIN
#endif
};

/* Decode one RGB565 pixel into integer HSV: hue [0,359], sat/val [0,255]. */
static void pixel_to_hsv(uint16 pixel, uint16 *hue, uint16 *saturation,
                         uint16 *value)
{
    /* SCC8660 is configured for RGB565 byte-swapped output. */
    uint16 rgb = (uint16)((pixel << 8) | (pixel >> 8));
    uint16 r5 = (rgb >> 11) & 0x1Fu;
    uint16 g6 = (rgb >> 5) & 0x3Fu;
    uint16 b5 = rgb & 0x1Fu;
    uint16 r = (uint16)((r5 * 255u + 15u) / 31u);
    uint16 g = (uint16)((g6 * 255u + 31u) / 63u);
    uint16 b = (uint16)((b5 * 255u + 15u) / 31u);
    uint16 max_value = r;
    uint16 min_value = r;
    uint16 delta;
    int32 h;

    if(g > max_value) max_value = g;
    if(b > max_value) max_value = b;
    if(g < min_value) min_value = g;
    if(b < min_value) min_value = b;
    delta = (uint16)(max_value - min_value);

    *value = max_value;
    if((max_value == 0u) || (delta == 0u))
    {
        *hue = 0u;
        *saturation = 0u;
        return;
    }

    *saturation = (uint16)(((uint32)delta * 255u) / max_value);
    if(max_value == r)
        h = (60 * ((int32)g - (int32)b)) / (int32)delta;
    else if(max_value == g)
        h = 120 + (60 * ((int32)b - (int32)r)) / (int32)delta;
    else
        h = 240 + (60 * ((int32)r - (int32)g)) / (int32)delta;
    if(h < 0)
        h += 360;
    *hue = (uint16)h;
}

static uint8 circle_target_pixel(uint16 pixel)
{
    uint16 hue;
    uint16 saturation;
    uint16 value;

    pixel_to_hsv(pixel, &hue, &saturation, &value);
    /* Reject black (value 0) and grey (saturation 0) pixels, as before. */
    if((value == 0u) || (saturation == 0u))
        return 0u;
    return (uint8)((hue >= g_threshold.h_min) &&
                   (hue <= g_threshold.h_max) &&
                   (saturation >= g_threshold.s_min) &&
                   (value >= g_threshold.v_min));
}

void blue_square_set_threshold(const color_threshold_t *thr)
{
    if(thr != NULL)
        g_threshold = *thr;
}

void blue_square_get_threshold(color_threshold_t *thr)
{
    if(thr != NULL)
        *thr = g_threshold;
}

/* Sample a small centred region of the frame and derive colour thresholds
   from it: hue min/max expanded by a margin, saturation/value minimums
   reduced by a margin.  The result is applied to the detector immediately. */
uint8 blue_square_sample_threshold(const uint16 *image, uint16 width,
                                   uint16 height, color_threshold_t *thr)
{
    uint16 cx;
    uint16 cy;
    uint16 x0;
    uint16 x1;
    uint16 y0;
    uint16 y1;
    uint16 x;
    uint16 y;
    uint16 h;
    uint16 s;
    uint16 v;
    uint16 h_min = 359u;
    uint16 h_max = 0u;
    uint16 s_min = 255u;
    uint16 v_min = 255u;

    if((image == NULL) || (thr == NULL) || (width < 3u) || (height < 3u))
        return 0u;

    cx = (uint16)(width / 2u);
    cy = (uint16)(height / 2u);
    x0 = (cx > SAMPLE_REGION_HALF) ? (uint16)(cx - SAMPLE_REGION_HALF) : 0u;
    x1 = ((cx + SAMPLE_REGION_HALF) < width) ?
         (uint16)(cx + SAMPLE_REGION_HALF) : (uint16)(width - 1u);
    y0 = (cy > SAMPLE_REGION_HALF) ? (uint16)(cy - SAMPLE_REGION_HALF) : 0u;
    y1 = ((cy + SAMPLE_REGION_HALF) < height) ?
         (uint16)(cy + SAMPLE_REGION_HALF) : (uint16)(height - 1u);

    for(y = y0; y <= y1; y++)
    {
        for(x = x0; x <= x1; x++)
        {
            pixel_to_hsv(image[(uint32)y * width + x], &h, &s, &v);
            if(h < h_min) h_min = h;
            if(h > h_max) h_max = h;
            if(s < s_min) s_min = s;
            if(v < v_min) v_min = v;
        }
    }

    thr->h_min = (h_min > SAMPLE_MARGIN_H) ? (uint16)(h_min - SAMPLE_MARGIN_H) : 0u;
    thr->h_max = (h_max < (359u - SAMPLE_MARGIN_H)) ?
                 (uint16)(h_max + SAMPLE_MARGIN_H) : 359u;
    thr->s_min = (s_min > SAMPLE_MARGIN_S) ? (uint16)(s_min - SAMPLE_MARGIN_S) : 0u;
    thr->v_min = (v_min > SAMPLE_MARGIN_V) ? (uint16)(v_min - SAMPLE_MARGIN_V) : 0u;

    g_threshold = *thr;
    return 1u;
}

static void blue_component_init(blue_component_t *comp, uint16 x, uint16 y)
{
    comp->area = 0;
    comp->sum_x = 0u;
    comp->sum_y = 0u;
    comp->x_min = x;
    comp->x_max = x;
    comp->y_min = y;
    comp->y_max = y;
    comp->width = 0;
    comp->height = 0;
    comp->fill_permille = 0;
    comp->aspect_permille = 0;
    comp->seed_x = x;
    comp->seed_y = y;
    comp->touch_border = 0;
    comp->min_sum = 32767;
    comp->max_sum = -32768;
    comp->min_diff = 32767;
    comp->max_diff = -32768;
    comp->tl_x = x;
    comp->tl_y = y;
    comp->tr_x = x;
    comp->tr_y = y;
    comp->br_x = x;
    comp->br_y = y;
    comp->bl_x = x;
    comp->bl_y = y;
}

static void blue_component_update(blue_component_t *comp, uint16 x, uint16 y)
{
    int16 sum;
    int16 diff;

    comp->area++;
    comp->sum_x += x;
    comp->sum_y += y;
    if(x < comp->x_min) comp->x_min = x;
    if(x > comp->x_max) comp->x_max = x;
    if(y < comp->y_min) comp->y_min = y;
    if(y > comp->y_max) comp->y_max = y;
    if((x == 0u) || (y == 0u) ||
       (x == (SCC8660_W - 1u)) || (y == (SCC8660_H - 1u)))
    {
        comp->touch_border = 1u;
    }

    /* The four diagonal extrema are the same corner construction used by
       the previously working detector. */
    sum = (int16)x + (int16)y;
    diff = (int16)x - (int16)y;
    if(sum < comp->min_sum)
    {
        comp->min_sum = sum;
        comp->tl_x = x;
        comp->tl_y = y;
    }
    if(sum > comp->max_sum)
    {
        comp->max_sum = sum;
        comp->br_x = x;
        comp->br_y = y;
    }
    if(diff > comp->max_diff)
    {
        comp->max_diff = diff;
        comp->tr_x = x;
        comp->tr_y = y;
    }
    if(diff < comp->min_diff)
    {
        comp->min_diff = diff;
        comp->bl_x = x;
        comp->bl_y = y;
    }
}

static void blue_component_finish(blue_component_t *comp)
{
    uint32 box_area;
    uint16 min_side;
    uint16 max_side;

    comp->width = (uint16)(comp->x_max - comp->x_min + 1u);
    comp->height = (uint16)(comp->y_max - comp->y_min + 1u);
    box_area = (uint32)comp->width * (uint32)comp->height;
    comp->fill_permille = (box_area > 0u) ?
        (uint16)(((uint32)comp->area * 1000u) / box_area) : 0u;

    if(comp->width < comp->height)
    {
        min_side = comp->width;
        max_side = comp->height;
    }
    else
    {
        min_side = comp->height;
        max_side = comp->width;
    }
    comp->aspect_permille = (max_side > 0u) ?
        (uint16)(((uint32)min_side * 1000u) / max_side) : 0u;
}

static uint8 blue_component_is_square_like(const blue_component_t *comp)
{
    if(comp->touch_border || comp->area < BLUE_MIN_AREA ||
       comp->area > BLUE_MAX_AREA)
    {
        return 0u;
    }
    if(comp->width < BLUE_MIN_SIZE || comp->height < BLUE_MIN_SIZE ||
       comp->fill_permille < BLUE_MIN_FILL_PERMILLE ||
       comp->fill_permille > BLUE_MAX_FILL_PERMILLE ||
       comp->aspect_permille < BLUE_MIN_ASPECT_PERMILLE)
    {
        return 0u;
    }
    return 1u;
}

static uint32 blue_component_score(const blue_component_t *comp)
{
    /* A filled circle is approximately 785 permille of its bounding box. */
    uint16 circle_fill_error = (comp->fill_permille > 785u) ?
        (uint16)(comp->fill_permille - 785u) :
        (uint16)(785u - comp->fill_permille);
    uint16 fill_quality = (circle_fill_error >= 785u) ? 0u :
        (uint16)(785u - circle_fill_error);
    return (uint32)fill_quality * 2u +
           (uint32)comp->aspect_permille * 3u +
           (uint32)comp->area / 10u;
}

static void blue_span_push(uint16 *top, uint16 y, uint16 left, uint16 right)
{
    if(*top < BLUE_SPAN_STACK_SIZE)
    {
        blue_span_stack[*top].y = y;
        blue_span_stack[*top].x_left = left;
        blue_span_stack[*top].x_right = right;
        (*top)++;
    }
}

static uint8 blue_span_pop(uint16 *top, blue_span_t *span)
{
    if(*top == 0u)
    {
        return 0u;
    }
    (*top)--;
    *span = blue_span_stack[*top];
    return 1u;
}

static void blue_scan_span(blue_component_t *comp, uint16 seed_x, uint16 seed_y,
                           uint16 *left_out, uint16 *right_out)
{
    uint16 x;
    uint16 left = seed_x;
    uint16 right = seed_x;

    while(left > 0u && blue_mask[seed_y][left - 1u] == 255u)
        left--;
    while((right + 1u) < SCC8660_W && blue_mask[seed_y][right + 1u] == 255u)
        right++;

    for(x = left; x <= right; x++)
    {
        blue_mask[seed_y][x] = BLUE_MARK_VISITED;
        blue_component_update(comp, x, seed_y);
    }
    *left_out = left;
    *right_out = right;
}

static void blue_check_row(blue_component_t *comp, uint16 row,
                           uint16 left_limit, uint16 right_limit,
                           uint16 *top)
{
    uint16 x = left_limit;
    uint16 left;
    uint16 right;

    while(x <= right_limit)
    {
        while(x <= right_limit && blue_mask[row][x] != 255u)
            x++;
        if(x <= right_limit)
        {
            blue_scan_span(comp, x, row, &left, &right);
            blue_span_push(top, row, left, right);
            x = (uint16)(right + 1u);
        }
    }
}

static void blue_flood_component(blue_component_t *comp,
                                 uint16 seed_x, uint16 seed_y)
{
    uint16 top = 0u;
    uint16 left;
    uint16 right;
    blue_span_t span;

    blue_scan_span(comp, seed_x, seed_y, &left, &right);
    blue_span_push(&top, seed_y, left, right);

    while(blue_span_pop(&top, &span))
    {
        if(span.y > 0u)
            blue_check_row(comp, (uint16)(span.y - 1u), span.x_left, span.x_right, &top);
        if((span.y + 1u) < SCC8660_H)
            blue_check_row(comp, (uint16)(span.y + 1u), span.x_left, span.x_right, &top);
    }
    blue_component_finish(comp);
}

void blue_square_detect(const uint16 *image, uint16 width, uint16 height,
                        blue_square_result_t *result)
{
    uint16 x;
    uint16 y;
    uint16 scan_w = (width > SCC8660_W) ? SCC8660_W : width;
    uint16 scan_h = (height > SCC8660_H) ? SCC8660_H : height;
    uint32 best_score = 0u;
    uint8 have_best = 0u;
    blue_component_t comp;
    blue_component_t best;

    memset(result, 0, sizeof(blue_square_result_t));
    for(y = 0u; y < scan_h; y++)
    {
        for(x = 0u; x < scan_w; x++)
            blue_mask[y][x] = circle_target_pixel(image[(uint32)y * width + x]) ? 255u : 0u;
    }

    for(y = 0u; y < scan_h; y++)
    {
        for(x = 0u; x < scan_w; x++)
        {
            if(blue_mask[y][x] == 255u)
            {
                blue_component_init(&comp, x, y);
                blue_flood_component(&comp, x, y);
                if(blue_component_is_square_like(&comp) &&
                   (!have_best || blue_component_score(&comp) > best_score))
                {
                    best = comp;
                    best_score = blue_component_score(&comp);
                    have_best = 1u;
                }
            }
        }
    }

    if(have_best)
    {
        uint16 frame_center_x = (uint16)(scan_w / 2u);
        uint16 vertical_pixels;
        result->found = 1u;
        result->x_min = best.x_min;
        result->x_max = best.x_max;
        result->y_min = best.y_min;
        result->y_max = best.y_max;
        result->center_x = (uint16)(best.sum_x / (uint32)best.area);
        result->center_y = (uint16)(best.sum_y / (uint32)best.area);
        result->area = best.area;
        result->score = (uint8)((best.fill_permille > 1000u) ? 100u :
                                ((best.fill_permille * 100u) / 1000u));
        result->tl_x = best.tl_x;
        result->tl_y = best.tl_y;
        result->tr_x = best.tr_x;
        result->tr_y = best.tr_y;
        result->br_x = best.br_x;
        result->br_y = best.br_y;
        result->bl_x = best.bl_x;
        result->bl_y = best.bl_y;
        result->vertical_length_px = best.height;
        result->angle_deg = (int16)result->center_x - (int16)frame_center_x;
        /* Estimate distance from the circle's vertical image diameter:
         * distance = real diameter * focal length / image diameter. */
        vertical_pixels = result->vertical_length_px;
        result->distance_mm = ((uint32)PURPLE_CIRCLE_REAL_DIAMETER_MM *
                               PURPLE_CIRCLE_FOCAL_LENGTH_PX +
                               vertical_pixels / 2u) /
                              (vertical_pixels ? vertical_pixels : 1u);
    }

    /* Flood-fill uses 128 as a visited marker. Restore selected pixels to
       white so blue_square_show_mask() remains useful for threshold tuning. */
    for(y = 0u; y < scan_h; y++)
        for(x = 0u; x < scan_w; x++)
            if(blue_mask[y][x] == BLUE_MARK_VISITED)
                blue_mask[y][x] = 255u;
}

void blue_square_show_mask(uint16 width, uint16 height)
{
    tft180_show_gray_image(0, 0, blue_mask[0], width, height,
                           width, height, 128u);
}

void blue_square_draw(const blue_square_result_t *result)
{
    uint16 frame_center_x = (uint16)(SCC8660_W / 2u);

    /* 画面中心蓝色线 */
    tft180_draw_line(frame_center_x, 0u, frame_center_x,
                     (uint16)(SCC8660_H - 1u), RGB565_BLUE);

    /* 左侧参考线：青色/浅蓝色半透明效果 */
    uint16 left_line_x = (frame_center_x > STEER_REFERENCE_OFFSET_PX) ?
                          (uint16)(frame_center_x - STEER_REFERENCE_OFFSET_PX) : 0u;
    tft180_draw_line(left_line_x, 0u, left_line_x,
                     (uint16)(SCC8660_H - 1u), RGB565_CYAN);

    /* 右侧参考线：青色/浅蓝色半透明效果 */
    uint16 right_line_x = (frame_center_x + STEER_REFERENCE_OFFSET_PX < SCC8660_W) ?
                           (uint16)(frame_center_x + STEER_REFERENCE_OFFSET_PX) : (uint16)(SCC8660_W - 1u);
    tft180_draw_line(right_line_x, 0u, right_line_x,
                     (uint16)(SCC8660_H - 1u), RGB565_CYAN);

    if(result->found)
    {
        tft180_draw_line(result->x_min, result->y_min, result->x_max, result->y_min, RGB565_GREEN);
        tft180_draw_line(result->x_max, result->y_min, result->x_max, result->y_max, RGB565_GREEN);
        tft180_draw_line(result->x_max, result->y_max, result->x_min, result->y_max, RGB565_GREEN);
        tft180_draw_line(result->x_min, result->y_max, result->x_min, result->y_min, RGB565_GREEN);

        if(result->center_x >= 2u && result->center_x + 2u < SCC8660_W)
            tft180_draw_line(result->center_x - 2u, result->center_y,
                             result->center_x + 2u, result->center_y, RGB565_GREEN);
        if(result->center_y >= 2u && result->center_y + 2u < SCC8660_H)
            tft180_draw_line(result->center_x, result->center_y - 2u,
                             result->center_x, result->center_y + 2u, RGB565_GREEN);

        tft180_show_string(0, 0, "X:");
        tft180_show_int(16, 0, result->angle_deg, 3);
        tft180_show_string(48, 0, "D:");
        tft180_show_uint(64, 0, result->distance_mm, 5);
    }
}

/* ====================================================================== *
 * Camera tuning application: threshold / exposure adjustment loop.
 *
 * Runtime tuning demo for the blue-square detector.  It was moved out of
 * user/cpu0_main.c so the main file only drives the top-level loop.
 *
 * Controls (all active-low, pulled up):
 *   KEY4  - toggle between normal and threshold-adjust mode
 *   KEY3  - (adjust mode) select H_MIN/H_MAX/S_MIN/V_MIN parameter
 *   KEY1  - (normal) increase camera exposure, (adjust) increase threshold
 *   KEY2  - (normal) decrease camera exposure, (adjust) decrease threshold
 * ====================================================================== */

#define DISPLAY_ORIGINAL_IMAGE  (1)
#define EXPOSURE_DEFAULT        (50u)
#define EXPOSURE_STEP           (5u)
#define EXPOSURE_MIN            (0u)
#define EXPOSURE_MAX            (65535u)
#define EXPOSURE_FLASH_PAGE     (15u)
#define EXPOSURE_FLASH_MAGIC    (0x45585031u)  /* "EXP1" */

/* Threshold parameter indices adjusted by the +/- buttons. */
#define PARAM_H_MIN     (0u)
#define PARAM_H_MAX     (1u)
#define PARAM_S_MIN     (2u)
#define PARAM_V_MIN     (3u)
#define PARAM_COUNT     (4u)
#define THRESHOLD_STEP  (5u)

static const char *const threshold_param_names[PARAM_COUNT] =
{
    "H_MIN", "H_MAX", "S_MIN", "V_MIN"
};

/* Camera tuning application state (static so the module stays self-contained). */
static uint16 g_exposure;
static uint8  g_adjust_mode;
static uint8  g_param_index;
static blue_square_result_t g_raw;
static blue_square_result_t g_filtered;
static square_kalman_filter_t g_filter;

/* Control source selection (toggled at runtime by KEY3 in normal mode):
   CONTROL_RAW    - raw detection + EMA/PD (Color_discirmination style)
   CONTROL_KALMAN - Kalman-filtered detection output */
#define CONTROL_RAW     (0u)
#define CONTROL_KALMAN  (1u)
static uint8 g_control_mode = CONTROL_RAW;

/* Debug-mode toggle via the rotary encoder button (P11_11, MENU_KEY_1):
   double-click (Encoder_Btn_Double) enters camera debug mode, long-press
   (Encoder_Btn_Long) returns to base (motion control) mode.  Double-click
   is detected in the 10ms PIT interrupt. */
uint8 g_debug_mode = 0;              /* 0 = base (motion), 1 = debug (tuning) */
static uint8 g_tracking_enable = 0;         /* base mode: 0 = car stopped, 1 = camera tracking */

/* Read the saved exposure from flash, returning the default when invalid. */
static uint16 exposure_load(void)
{
    uint32 saved[2];

    flash_read_page(0u, EXPOSURE_FLASH_PAGE, saved, 2u);
    if((saved[0] == EXPOSURE_FLASH_MAGIC) &&
       (saved[1] <= EXPOSURE_MAX))
        return (uint16)saved[1];

    return EXPOSURE_DEFAULT;
}

/* Persist the exposure value to flash. */
static void exposure_save(uint16 exposure)
{
    uint32 saved[2];

    saved[0] = EXPOSURE_FLASH_MAGIC;
    saved[1] = (uint32)exposure;
    flash_write_page(0u, EXPOSURE_FLASH_PAGE, saved, 2u);
}

static void exposure_decrease(uint16 *exposure)
{
    *exposure = (*exposure >= EXPOSURE_STEP) ?
                (uint16)(*exposure - EXPOSURE_STEP) : EXPOSURE_MIN;
    (void)scc8660_set_brightness(*exposure);
    exposure_save(*exposure);
}

static void exposure_increase(uint16 *exposure)
{
    *exposure = (*exposure <= EXPOSURE_MAX - EXPOSURE_STEP) ?
                (uint16)(*exposure + EXPOSURE_STEP) : EXPOSURE_MAX;
    (void)scc8660_set_brightness(*exposure);
    exposure_save(*exposure);
}

/* Return a pointer to the requested threshold field, plus its max value. */
static uint16 *threshold_field(color_threshold_t *thr, uint8 param, uint16 *max)
{
    switch(param)
    {
    case PARAM_H_MIN: *max = 359u; return &thr->h_min;
    case PARAM_H_MAX: *max = 359u; return &thr->h_max;
    case PARAM_S_MIN: *max = 255u; return &thr->s_min;
    case PARAM_V_MIN: *max = 255u; return &thr->v_min;
    default:          *max = 0u;   return NULL;
    }
}

/* Adjust a threshold field by +/- THRESHOLD_STEP, clamped to [0, max]. */
static void threshold_adjust(uint8 param, int8 direction)
{
    color_threshold_t thr;
    uint16 max;
    uint16 *field;

    blue_square_get_threshold(&thr);
    field = threshold_field(&thr, param, &max);
    if(field == NULL)
        return;

    if(direction > 0)
        *field = ((*field + THRESHOLD_STEP) <= max) ?
                 (uint16)(*field + THRESHOLD_STEP) : max;
    else
        *field = (*field >= THRESHOLD_STEP) ? (uint16)(*field - THRESHOLD_STEP) : 0u;

    blue_square_set_threshold(&thr);
}

/* Current value of a threshold field (for on-screen display). */
static uint16 threshold_field_value(uint8 param)
{
    color_threshold_t thr;
    uint16 max;
    uint16 *field;

    blue_square_get_threshold(&thr);
    field = threshold_field(&thr, param, &max);
    return (field != NULL) ? *field : 0u;
}

/* 时间确认：连续 TRACK_CONFIRM_FRAMES 帧都检测到目标才"锁定"，
   避免一闪而过的蓝色噪声（如背景里偶尔出现的蓝物体）被误当成目标。
   帧数越大越抗噪、但锁定越慢；建议 3~5。 */
#define TRACK_CONFIRM_FRAMES    (3u)
static uint8 g_confirm_count = 0;

/* Detect the target (raw + Kalman filtered). */
static void detect_target(blue_square_result_t *raw,
                          blue_square_result_t *filtered,
                          square_kalman_filter_t *filter)
{
    blue_square_detect(scc8660_image[0], SCC8660_W, SCC8660_H, raw);

    /* 时间确认：连续 N 帧都检测到才把 found 置 1；任一帧丢失就清零重新累计。 */
    if(raw->found)
    {
        if(g_confirm_count < TRACK_CONFIRM_FRAMES)
            g_confirm_count++;
    }
    else
    {
        g_confirm_count = 0;
    }
    if(g_confirm_count < TRACK_CONFIRM_FRAMES)
        raw->found = 0;   /* 未达到确认帧数，视为无目标 */

    *filtered = *raw;
    if(raw->found)
    {
        (void)square_kalman_update(filter, raw->angle_deg, raw->distance_mm,
                                   &filtered->angle_deg,
                                   &filtered->distance_mm);
    }
    else
    {
        square_kalman_init(filter);
    }
}

/* Render one frame: raw image / threshold mask depending on mode. */
static void display_frame(uint8 adjust_mode, uint8 param_index, uint8 control_mode,
                          uint8 tracking, uint16 exposure, const blue_square_result_t *raw)
{
    if(adjust_mode)
    {
        blue_square_show_mask(SCC8660_W, SCC8660_H);
        tft180_show_string(0, 0, "ADJ");
        tft180_show_string(40, 0, threshold_param_names[param_index]);
        tft180_show_uint(112, 0, threshold_field_value(param_index), 3);

        tft180_show_string(0, 112, "SEL:+/-");
    }
    else
    {
#if DISPLAY_ORIGINAL_IMAGE
        tft180_displayimage8660(scc8660_image[0], SCC8660_W, SCC8660_H);
        blue_square_draw(raw);
#else
        blue_square_show_mask(SCC8660_W, SCC8660_H);
#endif

        tft180_show_string(0, 112, "EXP:");
        tft180_show_uint(32, 112, exposure, 5);
    }

    /* Control source + tracking indicators. */
    tft180_show_string(0, 104, control_mode ? "KAL" : "RAW");
    tft180_show_string(48, 104, tracking ? "TRK" : "   ");
}

/* ===== Car control: drive the vehicle toward the detected target. ===== *
 * Mirrors the camera-control logic in Color_discirmination.c
 * (display_track_mode).  Steering is a PD controller on the horizontal
 * pixel offset (angle_deg), speed is derived from the estimated distance. */
#define REAR_CAMERA 0

/* Steering PD gains.  STEER_P_BOOST is applied only to the horizontal error
   beyond STEER_REFERENCE_OFFSET_PX (see blue_square_car_control).  Raise it
   to turn harder when the target is near the frame edge. */
#define STEER_P_NORMAL   (18)
#define STEER_P_BOOST    (80)
#define STEER_P_D        (8)

static void blue_square_car_control(const blue_square_result_t *raw,
                                    const blue_square_result_t *filtered,
                                    uint8 use_kalman,
                                    uint8 reset)
{
    static int16  filt_err    = 0;
    static int16  prev_err    = 0;
    static int16  last_speed  = 0;
    static uint16 lost_frames = 0;
    static uint16 dist_filt   = 300;
    static int16  last_steer_cmd = 0;  // ← 添加这一行，保存最后的转向命令

//    #define LOST_TIMEOUT  (15)   /* target lost N frames -> stop (~0.25s @60fps) */

    if(reset)
    {
        /* Stop the car and clear the control state (used when entering debug
           mode so motion control resumes cleanly on exit). */
        filt_err    = 0;
        prev_err    = 0;
        last_speed  = 0;
        lost_frames = 0;
        dist_filt   = 300;
        Angle_Target_Position = 0;
        Back_Motor_SetPwm(0, 0);
        return;
    }

    const blue_square_result_t *src = use_kalman ? filtered : raw;

    /* Horizontal error (px): left is negative, right is positive. */
    int16 cx_error = src->angle_deg;
    if (use_kalman)
        filt_err = cx_error;                        /* Kalman output already smoothed */
    else
        filt_err = (int16)(0.6 * filt_err + 0.4 * cx_error);
    int16 error_rate = filt_err - prev_err;
    prev_err = filt_err;

    /* Dual-slope steering: normal PD while the target centre stays between
       the two reference lines, boosted proportional gain once it crosses one.
       The switch is continuous at the line (both slopes give
       STEER_P_NORMAL * STEER_REFERENCE_OFFSET_PX there), so no steering step. */
    int16 abs_err = (filt_err > 0) ? filt_err : (int16)-filt_err;
    int16 steer_cmd;
    if (abs_err <= STEER_REFERENCE_OFFSET_PX)
    {
        steer_cmd = (int16)(STEER_P_NORMAL * filt_err + STEER_P_D * error_rate);
    }
    else
    {
        int16 sign   = (filt_err > 0) ? 1 : -1;
        int16 excess = (int16)(abs_err - STEER_REFERENCE_OFFSET_PX);
        steer_cmd = (int16)(STEER_P_NORMAL * (int16)STEER_REFERENCE_OFFSET_PX * sign +
                            STEER_P_BOOST * excess * sign +
                            STEER_P_D * error_rate);
    }
    if (filt_err > -3 && filt_err < 3) steer_cmd = 0;   /* deadband */
    if (steer_cmd > 937)  steer_cmd = 937;              /* clamp to steering range */
    if (steer_cmd < -937) steer_cmd = -937;

    /* Distance (mm).  Only refreshed while the target is visible so a short
       dropout keeps the last distance estimate. */
    if (src->found && src->distance_mm > 0) {
        if (use_kalman)
            dist_filt = (uint16)src->distance_mm;   /* already filtered */
        else
            dist_filt = (uint16)(0.7 * dist_filt + 0.3 * (float)src->distance_mm);
    }

//    if (src->found)
//    {
//        lost_frames = 0;
//
//        int16 speed;
//        if (dist_filt <= 100) speed = 0;
//        else if (dist_filt >= 800) speed = 450;
//        else speed = (int16)((dist_filt - 100) * 450 / 700);
//
//        #if REAR_CAMERA
//        steer_cmd = -steer_cmd;
//        speed = -speed;
//        #endif
//
//        last_steer_cmd = steer_cmd;
//        last_speed = (int16)(0.3 * last_speed + 0.7 * speed);
//        Angle_Target_Position = steer_cmd;
//        Back_Motor_SetPwm(last_speed, last_speed);
//    }
//    else if (lost_frames < LOST_TIMEOUT)
//    {
//        /* 目标短暂丢失 */
//        lost_frames++;
//
//        // 判断转向是否在中值附近（±100 以内认为是"直行/小弯"）
//        if (last_steer_cmd > -100 && last_steer_cmd < 100)
//        {
//            // 转向接近中值 → 静止等待
//            last_speed = 0;
//            Back_Motor_SetPwm(0, 0);
//        }
//        else
//        {
//            // 转向角度大 → 保持转向继续转弯
//            if (last_speed > 500) last_speed = 500;
//            if (last_speed < -500) last_speed = -500;
//            Angle_Target_Position = last_steer_cmd;
//            Back_Motor_SetPwm(last_speed, last_speed);
//        }
//    }
//    else
//    {
//        /* 目标长时间丢失：停车 */
//        lost_frames = LOST_TIMEOUT;
//        last_speed = 0;
//        last_steer_cmd = 0;
//        Back_Motor_SetPwm(0, 0);
//    }
    if (src->found)
    {
        lost_frames = 0;

        int16 speed;
        if (dist_filt <= 100) speed = 0;
        else if (dist_filt >= 600) speed = 450;
        else speed = (int16)((dist_filt - 100) * 450 / 700);

        #if REAR_CAMERA
        steer_cmd = -steer_cmd;
        speed = -speed;
        #endif

        last_steer_cmd = steer_cmd;
        last_speed = (int16)(0.3 * last_speed + 0.7 * speed);
        Angle_Target_Position = steer_cmd;
        Back_Motor_SetPwm(last_speed, last_speed);
    }
    else
    {
        /* 目标丢失：立即停车，转向归中 */
        lost_frames = 0;
        last_speed = 0;
//        last_steer_cmd = 0;
//        Angle_Target_Position = 0;
        Back_Motor_SetPwm(0, 0);
    }
}

/* Toggle the control source between raw (EMA/PD) and Kalman. */
static void blue_square_toggle_control_source(void)
{
    g_control_mode = (g_control_mode == CONTROL_RAW) ? CONTROL_KALMAN : CONTROL_RAW;
}

/* Rotary encoder button (P11_11): double-click enters debug mode, long-press
   exits back to base (motion control) mode. */
static void blue_square_handle_mode_switch(void)
{
    if(Encoder_Btn_Long)
    {
        Encoder_Btn_Long = 0;
        if(g_debug_mode)
        {
            g_debug_mode = 0;                       /* exit debug -> base motion */
            blue_square_car_control(&g_raw, &g_filtered, g_control_mode, 1);
        }
    }
}

void blue_square_camera_init(void)
{
    scc8660_init();

    g_exposure = exposure_load();
    square_kalman_init(&g_filter);
    (void)scc8660_set_brightness(g_exposure);

    g_adjust_mode = 0u;
    g_param_index = PARAM_H_MIN;
}

void blue_square_camera_process(void)
{
    if(!scc8660_finish_flag)
        return;

    scc8660_finish_flag = 0;
    // 【新增】修点模式下跳过所有图像处理，节省时间
        if(INS_edit_flag)
        {
            // 不显示图像，不检测目标，快速返回
            return;
        }
    if(g_raw.found && INS_Flag==1)
    {
        // 摄像头检测到目标
        Buzzer_check(500);
//        ips200_show_string(0, 16*7, "Target Found!");
    }

    blue_square_handle_mode_switch();   /* encoder double-click / long-press */

    detect_target(&g_raw, &g_filtered, &g_filter);

    if(g_debug_mode)
    {
        /* ===== Debug mode: camera tuning (car stopped) ===== */

        /* KEY4 toggles threshold-adjust sub-mode. */
        if(key4_flag)
        {
            key4_flag = 0;
            g_adjust_mode = (uint8)!g_adjust_mode;
        }

        if(g_adjust_mode)
        {
            if(key3_flag)
            {
                key3_flag = 0;
                g_param_index = (uint8)((g_param_index + 1u) % PARAM_COUNT);
            }
            if(key1_flag)
            {
                key1_flag = 0;
                threshold_adjust(g_param_index, 1);
            }
            if(key2_flag)
            {
                key2_flag = 0;
                threshold_adjust(g_param_index, -1);
            }
        }
        else
        {
            /* KEY3 toggles the control source (raw EMA/PD vs Kalman). */
            if(key3_flag)
            {
                key3_flag = 0;
                blue_square_toggle_control_source();
            }
            if(key2_flag)
            {
                key2_flag = 0;
                exposure_decrease(&g_exposure);
            }
            if(key1_flag)
            {
                key1_flag = 0;
                exposure_increase(&g_exposure);
            }
        }
    }
    else
    {
        /* ===== Base mode: motion control (follow the target) ===== */

        /* KEY1 toggles camera tracking on/off. */
        if(key2_flag)
        {
            Buzzer_check(50);
            key2_flag = 0;
            Motor_Start = 1;    // 启动电机
//            Transform = 1;
            Camera_Navigation_Motor_flag = 0;
            g_tracking_enable = (uint8)!g_tracking_enable;
            if(!g_tracking_enable)
                blue_square_car_control(&g_raw, &g_filtered, g_control_mode, 1);  /* stop + reset */
        }

        if(g_tracking_enable && Transform == 1 && Camera_Navigation_Motor_flag ==0)
        {
            blue_square_car_control(&g_raw, &g_filtered, g_control_mode, 0);
        }
        else if(INS_Flag != 3 && g_tracking_enable==0 && Transform == 1 && Camera_Navigation_Motor_flag ==0)
        {
            /* Tracking off -> keep the car stopped and steering centred. */
            Angle_Target_Position = 0;
            Back_Motor_SetPwm(0, 0);
        }
    }
//    if(g_tracking_enable==0)
//    {
        display_frame(g_debug_mode ? g_adjust_mode : 0u,
                      g_param_index, g_control_mode, g_tracking_enable, g_exposure, &g_raw);
//    }
}
