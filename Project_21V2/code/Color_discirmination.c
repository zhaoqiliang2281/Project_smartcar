/*
 * @file    Color_discirmination.c
 * @brief   HSV 距离图 + 十字扩张腐蚀 目标识别
 *
 * 算法流程：
 * 1. RGB565 -> HSV (纯整数)
 * 2. 计算像素与目标 HSV 的加权距离 -> 相似度
 * 3. 全图稀疏扫描找最高相似度种子
 * 4. 十字扩张 + 腐蚀迭代 -> 精确中心坐标
 *
 * 特点：软距离匹配替代硬阈值，暴晒/阴影/背光均适用
 */

#include "Color_discirmination.h"

#pragma section all "cpu0_dsram"

// ---- 全局变量 ----
color_blob_t         g_blob;
color_learn_result_t g_learn_result;
env_light_level_t    g_env_level = ENV_NORMAL;
uint8                g_swap_rgb565 = 1;
uint16               g_camera_brightness = 200;

// ---- 多槽位目标 HSV 存储 ----
#define SLOT_COUNT 3
static hsv_color_t g_target_hsv[SLOT_COUNT];   // 学习到的目标 HSV
static uint8       g_slot_learned[SLOT_COUNT] = {0};
static uint8       g_current_slot = 0;
static uint8       g_any_slot_learned = 0;

// 默认目标 HSV（红色锥桶，用于初始检测）
static hsv_color_t g_default_hsv = {
    15,     // H: red ~21deg (0-255 = 0-360deg)
    180,    // S: moderately saturated
    180     // V: medium brightness
};

// 兼容旧类型（不再使用硬阈值，保留声明避免编译错误）
lab_threshold_t g_lab_thresh = { 0,255, 0,255, 0,255 };

// ---- 内部状态 ----
#pragma section all "cpu0_dsram"
static uint16       scc8660_image_save[SCC8660_H][SCC8660_W];
#pragma section all restore
static work_mode_t  g_mode = MODE_LEARN;
static color_blob_t g_prev_blob;
static uint8        g_track_initialized = 0;

// ---- HSV 距离权重 ----
#define W_H  (2)    // H 权重（色相最重要）
#define W_S  (1)    // S 权重
#define W_V  (1)    // V 权重

// ---- 扩张腐蚀参数 ----
#define SIM_THRESH_HIGH  (180)   // 初始相似度阈值（0-255，越高越严格）
#define SIM_THRESH_LOW   (100)   // 最低相似度阈值
#define EXPAND_MAX_ITER  (6)     // 最大扩张腐蚀迭代次数
#define CONVERGE_THRESH  (3)     // 中心收敛阈值（像素）


// =========================================================================
// 1. RGB -> HSV (纯整数)
// =========================================================================

void rgb565_to_rgb888(uint16 rgb, uint8 *r, uint8 *g, uint8 *b) {
    if (g_swap_rgb565) { rgb = (rgb >> 8) | (rgb << 8); }
    uint8 r5 = (rgb >> 11) & 0x1F;
    uint8 g6 = (rgb >> 5)  & 0x3F;
    uint8 b5 =  rgb        & 0x1F;
    *r = (r5 << 3) | (r5 >> 2);
    *g = (g6 << 2) | (g6 >> 4);
    *b = (b5 << 3) | (b5 >> 2);
}

// HSV: la=V, aa=H, bb=S (映射到旧结构体字段名)
void rgb888_to_lab(uint8 r, uint8 g, uint8 b, uint8 *la, uint8 *aa, uint8 *bb) {
    // Find max & min
    uint8 max = r > g ? (r > b ? r : b) : (g > b ? g : b);
    uint8 min = r < g ? (r < b ? r : b) : (g < b ? g : b);
    uint16 delta = (uint16)(max - min);

    // V = max (brightness)
    *la = max;

    // S = (delta * 255) / max  (0 if max==0)
    if (max == 0 || delta == 0) {
        *aa = 0;  // H undefined
        *bb = 0;  // S = 0
        return;
    }
    *bb = (uint8)((delta * 255u) / max);

    // H (0-360 -> 0-255)
    int16 hue;
    if (max == r) {
        hue = (int16)(60 * ((int16)(g) - (int16)(b)) / (int16)delta);
    } else if (max == g) {
        hue = (int16)(120 + 60 * ((int16)(b) - (int16)(r)) / (int16)delta);
    } else {
        hue = (int16)(240 + 60 * ((int16)(r) - (int16)(g)) / (int16)delta);
    }
    if (hue < 0) hue += 360;
    *aa = (uint8)((hue * 255) / 360);
}


// =========================================================================
// 2. HSV 加权距离 (0-255, 越小越接近)
// =========================================================================

static inline uint8 hsv_distance_u8(uint8 h, uint8 s, uint8 v,
                                     const hsv_color_t *tgt) {
    // H 环形距离
    int16 hd = (int16)h - (int16)tgt->h;
    if (hd < 0) hd = -hd;
    if (hd > 127) hd = 255 - hd;

    // S 距离
    int16 sd = (int16)s - (int16)tgt->s;
    if (sd < 0) sd = -sd;

    // V 距离
    int16 vd = (int16)v - (int16)tgt->v;
    if (vd < 0) vd = -vd;

    // 加权和
    int16 d = hd * W_H + sd * W_S + vd * W_V;
    if (d > 255) d = 255;
    return (uint8)d;
}

// 相似度 = 255 - 距离（越大越接近目标）
static inline uint8 hsv_similarity(uint8 h, uint8 s, uint8 v,
                                    const hsv_color_t *tgt) {
    return (uint8)(255 - hsv_distance_u8(h, s, v, tgt));
}

// 获取像素 (x,y) 相对于最佳匹配槽位的相似度
static uint8 pixel_similarity(const uint16 image[SCC8660_H][SCC8660_W],
                               uint16 x, uint16 y) {
    uint16 rgb = image[y][x];
    uint8 r, g, b;
    rgb565_to_rgb888(rgb, &r, &g, &b);
    uint8 h, s, v;
    rgb888_to_lab(r, g, b, &v, &h, &s);  // V=la, H=aa, S=bb

    // 如果还没学过，用默认值
    if (!g_any_slot_learned) {
        return hsv_similarity(h, s, v, &g_default_hsv);
    }

    // 取所有槽位中最高相似度
    uint8 best = 0;
    for (uint8 i = 0; i < SLOT_COUNT; i++) {
        if (g_slot_learned[i]) {
            uint8 sim = hsv_similarity(h, s, v, &g_target_hsv[i]);
            if (sim > best) best = sim;
        }
    }
    return best;
}


// =========================================================================
// 3. 学习：采样中心区域，平均 HSV
// =========================================================================

void sample_multi_point_lab(const uint16 image[SCC8660_H][SCC8660_W]) {
    uint16 cx = SCC8660_W / 2;
    uint16 cy = SCC8660_H / 2;
    uint32 h_sum = 0, s_sum = 0, v_sum = 0;
    uint8  cnt = 0;

    // 3x3 区域采样
    for (int8 dy = -8; dy <= 8; dy += 8) {
        for (int8 dx = -8; dx <= 8; dx += 8) {
            int16 sx = (int16)cx + dx;
            int16 sy = (int16)cy + dy;
            if (sx < 0 || sx >= SCC8660_W || sy < 0 || sy >= SCC8660_H) continue;

            uint16 rgb = image[(uint16)sy][(uint16)sx];
            uint8 r, g, b;
            rgb565_to_rgb888(rgb, &r, &g, &b);
            uint8 h, s, v;
            rgb888_to_lab(r, g, b, &v, &h, &s);

            // 只纳入有色彩的点（S > 30）
            if (s > 30) {
                h_sum += h; s_sum += s; v_sum += v;
                cnt++;
            }
        }
    }

    if (cnt < 3) {
        // 回退到单点
        uint16 rgb = image[cy][cx];
        uint8 r, g, b;
        rgb565_to_rgb888(rgb, &r, &g, &b);
        uint8 h_fb, s_fb, v_fb;
        rgb888_to_lab(r, g, b, &v_fb, &h_fb, &s_fb);
        h_sum = h_fb; s_sum = s_fb; v_sum = v_fb;
        cnt = 1;
    } else {
        g_target_hsv[g_current_slot].h = (uint8)(h_sum / cnt);
        g_target_hsv[g_current_slot].s = (uint8)(s_sum / cnt);
        g_target_hsv[g_current_slot].v = (uint8)(v_sum / cnt);
    }

    g_slot_learned[g_current_slot] = 1;
    g_any_slot_learned = 1;

    // 更新兼容字段（用于屏幕显示）
    g_learn_result.a_median = g_target_hsv[g_current_slot].h;
    g_learn_result.b_median = g_target_hsv[g_current_slot].s;
    g_learn_result.l_median = g_target_hsv[g_current_slot].v;
    g_learn_result.avg_brightness = estimate_global_brightness(image);
    g_env_level = detect_env_light(image);
}

void sample_region_lab(const uint16 image[SCC8660_H][SCC8660_W],
                       uint16 cx, uint16 cy, uint16 half_w, uint16 half_h) {
    // Fallback to center sampling for simplicity
    sample_multi_point_lab(image);
}


// =========================================================================
// 4. 全局亮度 & 环境检测（不变）
// =========================================================================

uint16 estimate_global_brightness(const uint16 image[SCC8660_H][SCC8660_W]) {
    #define BR_STEP (8)
    uint32 sum = 0; uint16 cnt = 0;
    for (uint16 y = 0; y < SCC8660_H; y += BR_STEP) {
        for (uint16 x = 0; x < SCC8660_W; x += BR_STEP) {
            uint16 rgb = image[y][x];
            if (g_swap_rgb565) rgb = (rgb >> 8) | (rgb << 8);
            uint8 r = ((rgb >> 11) & 0x1F) << 3;
            uint8 g = ((rgb >> 5)  & 0x3F) << 2;
            uint8 b = ( rgb        & 0x1F) << 3;
            sum += (uint32)((uint16)r * 77 + (uint16)g * 150 + (uint16)b * 29) / 256;
            cnt++;
        }
    }
    return (cnt > 0) ? (uint16)(sum / cnt) : 128;
}

env_light_level_t detect_env_light(const uint16 image[SCC8660_H][SCC8660_W]) {
    uint16 br = estimate_global_brightness(image);
    if (br > 210) return ENV_BRIGHT;
    if (br > 150) return ENV_NORMAL;
    if (br > 80)  return ENV_OVERCAST;
    return ENV_SHADOW;
}

void adapt_threshold_to_env(env_light_level_t env, lab_threshold_t *thresh,
                            const color_learn_result_t *learn) {
    (void)env; (void)thresh; (void)learn;
    // HSV 距离匹配自带环境适应性，不需要额外调整
}


// =========================================================================
// 5. 找种子：稀疏扫描全图，找最高相似度像素
// =========================================================================

static uint8 find_seed_full(const uint16 image[SCC8660_H][SCC8660_W],
                             uint16 *seed_x, uint16 *seed_y, uint8 *best_sim) {
    uint8  best = 0;
    uint16 bx = SCC8660_W / 2, by = SCC8660_H / 2;

    for (uint16 y = 0; y < SCC8660_H; y += 2) {
        for (uint16 x = 0; x < SCC8660_W; x += 2) {
            uint8 sim = pixel_similarity(image, x, y);
            if (sim > best) {
                best = sim;
                bx = x; by = y;
            }
        }
    }
    *seed_x = bx;
    *seed_y = by;
    *best_sim = best;
    return (uint8)(best >= SIM_THRESH_LOW ? 1 : 0);
}

// 在窗口内找种子
static uint8 find_seed_window(const uint16 image[SCC8660_H][SCC8660_W],
                               int32 x0, int32 x1, int32 y0, int32 y1,
                               uint16 *seed_x, uint16 *seed_y, uint8 *best_sim) {
    uint8  best = 0;
    uint16 bx = (uint16)((x0 + x1) / 2), by = (uint16)((y0 + y1) / 2);

    for (int32 y = y0; y < y1; y++) {
        for (int32 x = x0; x < x1; x++) {
            if (x < 0 || x >= SCC8660_W || y < 0 || y >= SCC8660_H) continue;
            uint8 sim = pixel_similarity(image, (uint16)x, (uint16)y);
            if (sim > best) {
                best = sim;
                bx = (uint16)x; by = (uint16)y;
            }
        }
    }
    *seed_x = bx;
    *seed_y = by;
    *best_sim = best;
    return (uint8)(best >= SIM_THRESH_LOW ? 1 : 0);
}


// =========================================================================
// 6. 十字扩张 + 腐蚀迭代
// =========================================================================

// 扫描水平线，从 x 向 dir(-1=左, 1=右) 方向找到第一个相似度低于阈值的边界
static uint16 scan_horizontal(const uint16 image[SCC8660_H][SCC8660_W],
                               uint16 x, uint16 y, int8 dir, uint8 thresh) {
    int16 xi = (int16)x;
    while (xi >= 0 && xi < SCC8660_W) {
        if (pixel_similarity(image, (uint16)xi, y) < thresh) break;
        xi += dir;
    }
    // 回退一步（回到有效像素）
    xi -= dir;
    if (xi < 0) xi = 0;
    if (xi >= SCC8660_W) xi = SCC8660_W - 1;
    return (uint16)xi;
}

static uint16 scan_vertical(const uint16 image[SCC8660_H][SCC8660_W],
                             uint16 x, uint16 y, int8 dir, uint8 thresh) {
    int16 yi = (int16)y;
    while (yi >= 0 && yi < SCC8660_H) {
        if (pixel_similarity(image, x, (uint16)yi) < thresh) break;
        yi += dir;
    }
    yi -= dir;
    if (yi < 0) yi = 0;
    if (yi >= SCC8660_H) yi = SCC8660_H - 1;
    return (uint16)yi;
}

static void cross_expand_erode(const uint16 image[SCC8660_H][SCC8660_W],
                                uint16 *cx, uint16 *cy,
                                uint16 *left, uint16 *right,
                                uint16 *top, uint16 *bottom) {
    uint8 thresh = SIM_THRESH_HIGH;
    uint16 prev_cx = *cx, prev_cy = *cy;

    for (uint8 iter = 0; iter < EXPAND_MAX_ITER; iter++) {
        // ---- 十字扩张：从中心向四方向扩展 ----
        uint16 l = scan_horizontal(image, *cx, *cy, -1, thresh);
        uint16 r = scan_horizontal(image, *cx, *cy,  1, thresh);
        uint16 t = scan_vertical(image, *cx, *cy, -1, thresh);
        uint16 b = scan_vertical(image, *cx, *cy,  1, thresh);

        // ---- 腐蚀：从边界向内收缩到有信号的区域 ----
        // 上下边界向内腐蚀
        while (t < *cy && pixel_similarity(image, *cx, t) < thresh) t++;
        while (b > *cy && pixel_similarity(image, *cx, b) < thresh) b--;

        // 左右边界向内腐蚀（在中间行检测）
        uint16 mid_y = (t + b) / 2;
        while (l < *cx && pixel_similarity(image, l, mid_y) < thresh) l++;
        while (r > *cx && pixel_similarity(image, r, mid_y) < thresh) r--;

        // 更新中心
        uint16 new_cx = (l + r) / 2;
        uint16 new_cy = (t + b) / 2;

        // 检查收敛
        int16 dx = (int16)new_cx - (int16)*cx;
        int16 dy = (int16)new_cy - (int16)*cy;
        if (dx < 0) dx = -dx; if (dy < 0) dy = -dy;

        *cx = new_cx; *cy = new_cy;
        *left = l; *right = r; *top = t; *bottom = b;

        if (dx <= CONVERGE_THRESH && dy <= CONVERGE_THRESH) break;

        // 逐步放宽阈值（适应边缘模糊区域）
        if (iter >= 3 && thresh > SIM_THRESH_LOW) {
            thresh -= 15;
            if (thresh < SIM_THRESH_LOW) thresh = SIM_THRESH_LOW;
        }

        prev_cx = *cx; prev_cy = *cy;
    }
}


// =========================================================================
// 7. 色块检测（HSV 距离 + 扩张腐蚀）
// =========================================================================

void find_color_blob(const uint16 image[SCC8660_H][SCC8660_W], color_blob_t *blob) {
    uint16 seed_x, seed_y;
    uint8  best_sim;

    if (!find_seed_full(image, &seed_x, &seed_y, &best_sim)) {
        blob->valid = 0;
        return;
    }

    uint16 cx = seed_x, cy = seed_y;
    uint16 left, right, top, bottom;
    cross_expand_erode(image, &cx, &cy, &left, &right, &top, &bottom);

    uint16 w = right - left;
    uint16 h = bottom - top;

    // 过滤太小或太大的区域
    if (w < 3 || h < 3 || w > (SCC8660_W * 3 / 4) || h > (SCC8660_H * 3 / 4)) {
        blob->valid = 0;
        return;
    }

    blob->valid  = 1;
    blob->cx     = cx;
    blob->cy     = cy;
    blob->left   = left;
    blob->right  = right;
    blob->top    = top;
    blob->bottom = bottom;
    blob->area   = (uint32)(w * h);
}

void find_color_blob_fast(const uint16 image[SCC8660_H][SCC8660_W],
                           color_blob_t *blob, const color_blob_t *prev_blob) {
    if (prev_blob == NULL || !prev_blob->valid) {
        find_color_blob(image, blob);
        return;
    }

    // 以上一帧中心为基准，2x 窗口搜索
    uint16 pw = prev_blob->right - prev_blob->left;
    uint16 ph = prev_blob->bottom - prev_blob->top;
    int32 x0 = (int32)prev_blob->cx - (int32)(pw > 0 ? pw : 20);
    int32 x1 = (int32)prev_blob->cx + (int32)(pw > 0 ? pw : 20);
    int32 y0 = (int32)prev_blob->cy - (int32)(ph > 0 ? ph : 20);
    int32 y1 = (int32)prev_blob->cy + (int32)(ph > 0 ? ph : 20);

    uint16 seed_x, seed_y;
    uint8  best_sim;

    if (!find_seed_window(image, x0, x1, y0, y1, &seed_x, &seed_y, &best_sim)) {
        // 窗口内丢失，全图搜索
        find_color_blob(image, blob);
        return;
    }

    uint16 cx = seed_x, cy = seed_y;
    uint16 left, right, top, bottom;
    cross_expand_erode(image, &cx, &cy, &left, &right, &top, &bottom);

    uint16 w = right - left;
    uint16 h = bottom - top;

    if (w < 3 || h < 3 || w > (SCC8660_W * 3 / 4) || h > (SCC8660_H * 3 / 4)) {
        blob->valid = 0;
        return;
    }

    blob->valid  = 1;
    blob->cx     = cx;
    blob->cy     = cy;
    blob->left   = left;
    blob->right  = right;
    blob->top    = top;
    blob->bottom = bottom;
    blob->area   = (uint32)(w * h);
}


// =========================================================================
// 8. Camera / Screen / Keys / Display（保持原有结构）
// =========================================================================

void camera_init_and_config(void) {
    while (1) {
        if (scc8660_init()) { gpio_toggle_level(LED1); }
        else { break; }
        system_delay_ms(1000);
    }
    scc8660_set_reg(SCC8660_COLOR_MODE, 1);
    scc8660_set_reg(SCC8660_AUTO_EXP, 1);     // Auto exposure ON
    system_delay_ms(50);
    scc8660_set_reg(SCC8660_MANUAL_WB, 0);
    system_delay_ms(50);
    scc8660_set_brightness(g_camera_brightness);
    system_delay_ms(50);
}

void screen_init_and_welcome(void) {
    ips200_init(IPS200_TYPE_SPI);
    ips200_clear();
    ips200_set_color(RGB565_BLACK, RGB565_WHITE);
    ips200_show_string(0, 0,  "HSV Dist v4.0");
    ips200_show_string(0, 16, "MODE: LEARN");
    ips200_show_string(0, 32, "Press KEY1 to learn color");
}

static void handle_keys(void) {
    static uint8 key1_last = 1, key2_last = 1, key3_last = 1;
    uint8 key1_now = key1_state, key2_now = key2_state, key3_now = key3_state;

    if (key1_last == 1 && key1_now == 0) {
        sample_multi_point_lab(scc8660_image_save);
        g_current_slot = (g_current_slot + 1) % SLOT_COUNT;
        ips200_show_string(0, 16, "MODE: LEARN (slot saved)");
    }
    key1_last = key1_now;

    if (key2_last == 1 && key2_now == 0) {
        g_mode = MODE_LEARN;
        g_track_initialized = 0;
        ips200_show_string(0, 16, "MODE: LEARN               ");
    }
    key2_last = key2_now;

    if (key3_last == 1 && key3_now == 0) {
        if (g_any_slot_learned) {
            g_mode = MODE_TRACK;
            g_track_initialized = 0;
            ips200_show_string(0, 16, "MODE: TRACK                ");
        }
    }
    key3_last = key3_now;
}

static void display_env_level(void) {
    static const char *env_names[] = {"NORM", "BRIGHT", "SHADOW", "CLOUD"};
    uint8 eidx = (uint8)g_env_level;
    if (eidx > 3) eidx = 0;
    ips200_set_color(RGB565_BLACK, RGB565_WHITE);
    ips200_show_string(160, 0, (char *)env_names[eidx]);
}

static void display_learn_mode(const uint16 image[SCC8660_H][SCC8660_W]) {
    uint16 cx = SCC8660_W / 2, cy = SCC8660_H / 2;

    // Center pixel HSV
    uint16 rgb = image[cy][cx];
    uint8 r, g, b;
    rgb565_to_rgb888(rgb, &r, &g, &b);
    uint8 h, s, v;
    rgb888_to_lab(r, g, b, &v, &h, &s);

    ips200_set_color(RGB565_BLACK, RGB565_WHITE);
    ips200_show_string(0, 16 * 12, "CENTER HSV:");
    ips200_show_uint(0,   16 * 13, h, 3);
    ips200_show_uint(55,  16 * 13, s, 3);
    ips200_show_uint(110, 16 * 13, v, 3);

    // Target HSV (if learned)
    if (g_any_slot_learned) {
        uint8 si = (g_current_slot > 0) ? (g_current_slot - 1) : (SLOT_COUNT - 1);
        ips200_show_string(0, 16 * 14, "Target HSV:");
        ips200_show_uint(0,   16 * 15, g_target_hsv[si].h, 3);
        ips200_show_uint(55,  16 * 15, g_target_hsv[si].s, 3);
        ips200_show_uint(110, 16 * 15, g_target_hsv[si].v, 3);
    } else {
        ips200_show_string(0, 16 * 14, "Target: DEFAULT");
    }

    // 诊断：全图最高相似度
    uint16 sx, sy; uint8 bs;
    if (find_seed_full(image, &sx, &sy, &bs)) {
        ips200_set_color(RGB565_GREEN, RGB565_WHITE);
    } else {
        ips200_set_color(RGB565_RED, RGB565_WHITE);
    }
    ips200_show_string(0, 16 * 16, "MaxSim:");
    ips200_show_uint(55, 16 * 16, bs, 3);
    ips200_show_string(85, 16 * 16, "@");
    ips200_show_uint(95, 16 * 16, sx, 3);
    ips200_show_uint(130, 16 * 16, sy, 3);

    ips200_draw_line(cx - 8, cy, cx + 8, cy, RGB565_RED);
    ips200_draw_line(cx, cy - 8, cx, cy + 8, RGB565_RED);
}

static void display_track_mode(const uint16 image[SCC8660_H][SCC8660_W]) {
    if (g_track_initialized && g_prev_blob.valid) {
        find_color_blob_fast(image, &g_blob, &g_prev_blob);
    } else {
        find_color_blob(image, &g_blob);
        g_track_initialized = 1;
    }
    g_prev_blob = g_blob;

    #define REAR_CAMERA 1
    {
        static int16 filt_err = 0, prev_err = 0;
        static uint16 lost_frames = 0;      // 连续丢失帧计数
        static int16 last_speed = 0;        // 最后有效速度（丢失时保持）

        int16 cx_error = (int16)g_blob.cx - (SCC8660_W / 2);
        filt_err = (int16)(0.6 * filt_err + 0.4 * cx_error);
        int16 error_rate = filt_err - prev_err;
        prev_err = filt_err;
        int16 steer_cmd = 12 * filt_err + 5 * error_rate;
        if (filt_err > -3 && filt_err < 3) steer_cmd = 0;
        if (steer_cmd > 937)  steer_cmd = 937;
        if (steer_cmd < -937) steer_cmd = -937;

        static uint16 dist_filt = 300;
        uint16 blob_w = g_blob.right - g_blob.left;
        if (g_blob.valid && blob_w > 3) {
            uint16 dist_raw = (uint16)(5000u / blob_w);
            dist_filt = (uint16)(0.7 * dist_filt + 0.3 * dist_raw);
        }

//        #define LOST_TIMEOUT  (15)   // 连续丢失N帧后才停车（~0.25s @60fps）

        if (g_blob.valid) {
            lost_frames = 0;
            dispatch_car = 1;
            int16 speed;
            if (dist_filt <= 50) speed = 0;
            else if (dist_filt >= 400) speed = 300;
            else speed = (int16)(dist_filt - 50) * 300 / 350;
            int16 abs_steer = (steer_cmd > 0) ? steer_cmd : -steer_cmd;
            speed = (int16)((int32)speed * (1874 - abs_steer) / 1874);
            #if REAR_CAMERA
            steer_cmd = -steer_cmd; speed = -speed;
            #endif
            // Speed smoothing: gradual ramp up/down (EMA)
            last_speed = (int16)(0.5 * last_speed + 0.5 * speed);
            Angle_Target_Position = steer_cmd;
            Back_Motor_SetPwm(last_speed, last_speed);
        } else if (lost_frames < LOST_TIMEOUT) {
            // Brief loss: keep last speed + steering, count frames
            lost_frames++;
            if (lost_frames >= 8) {
                // Gradually slow down during extended loss
                last_speed = (int16)(last_speed * 0.7);
                if (last_speed > -20 && last_speed < 20) last_speed = 0;
            }
            Back_Motor_SetPwm(last_speed, last_speed);
        } else {
            // Target truly lost: stop
            lost_frames = LOST_TIMEOUT;
            last_speed = 0;
            Back_Motor_SetPwm(0, 0);
        }

        ips200_set_color(RGB565_BLACK, RGB565_WHITE);
        ips200_show_string(0, 16 * 10, "Dist:");
        ips200_show_uint(45, 16 * 10, dist_filt, 4);
        ips200_show_string(80, 16 * 10, "cm W:");
        ips200_show_uint(126, 16 * 10, blob_w, 3);
    }

    // 显示目标 HSV
    ips200_set_color(RGB565_BLACK, RGB565_WHITE);
    ips200_show_string(0, 16 * 12, "Target H:");
    ips200_show_uint(55,  16 * 12, g_target_hsv[(g_current_slot > 0 ? g_current_slot-1 : SLOT_COUNT-1)].h, 3);
    ips200_show_string(0, 16 * 13, "Target S:");
    ips200_show_uint(55,  16 * 13, g_target_hsv[(g_current_slot > 0 ? g_current_slot-1 : SLOT_COUNT-1)].s, 3);
    ips200_show_string(120, 16 * 12, "DET:");
    ips200_show_uint(165, 16 * 12, g_blob.valid ? 1 : 0, 1);

    ips200_set_color(RGB565_BLACK, RGB565_WHITE);
    if (g_blob.valid) {
        uint16 color = RGB565_RED;
        ips200_draw_line(g_blob.left,  g_blob.top,    g_blob.right, g_blob.top,    color);
        ips200_draw_line(g_blob.left,  g_blob.bottom, g_blob.right, g_blob.bottom, color);
        ips200_draw_line(g_blob.left,  g_blob.top,    g_blob.left,  g_blob.bottom, color);
        ips200_draw_line(g_blob.right, g_blob.top,    g_blob.right, g_blob.bottom, color);
        ips200_show_string(0, 16 * 11, "cx:   cy:   area:  ");
        ips200_show_uint(25,  16 * 11, g_blob.cx, 3);
        ips200_show_uint(80,  16 * 11, g_blob.cy, 3);
        ips200_show_uint(140, 16 * 11, (uint16)g_blob.area, 4);
        ips200_show_string(0, 16 * 9, "Steer:");
        ips200_show_int(55, 16 * 9, (int16)Angle_Target_Position, 5);
    } else {
        ips200_show_string(0, 16 * 11, "              NO DETECT");
    }
}

// =========================================================================
// 9. Public API
// =========================================================================

void vision_init(void) {
    camera_init_and_config();
    screen_init_and_welcome();
}

void vision_process_frame(void) {
    static uint16 frame_count = 0;
    static uint32 fps_last_time_us = 0;
    static uint16 fps_frame_cnt = 0;
    static float  current_fps = 0.0f;

    if (!scc8660_finish_flag) {
        if (++frame_count % 100 == 0) {
            ips200_set_color(RGB565_RED, RGB565_WHITE);
            ips200_show_string(0, 120, "CAM: no frame");
        }
        return;
    }
    frame_count++;

    // FPS
    {
        uint32 now_us = system_getval_us();
        fps_frame_cnt++;
        if (fps_last_time_us == 0) {
            fps_last_time_us = now_us; fps_frame_cnt = 0;
        } else {
            uint32 elapsed = (now_us >= fps_last_time_us) ? (now_us - fps_last_time_us)
                : (0xFFFFFFFF - fps_last_time_us + now_us + 1);
            if (elapsed >= 1000000) {
                current_fps = (float)fps_frame_cnt * 1000000.0f / (float)elapsed;
                fps_frame_cnt = 0; fps_last_time_us = now_us;
            }
        }
    }

    gpio_toggle_level(LED1);
    memcpy(scc8660_image_save[0], scc8660_image[0], SCC8660_IMAGE_SIZE);
    scc8660_finish_flag = 0;

    handle_keys();

    menu_encoder_update();
    if (menu_encoder_has_change()) {
        int8 enc = (int8)menu_encoder_get_change();
        if (enc > 0 && g_camera_brightness < 550) g_camera_brightness += 20;
        else if (enc < 0 && g_camera_brightness > 20) g_camera_brightness -= 20;
        scc8660_set_brightness(g_camera_brightness);
    }

    ips200_show_rgb565_image(0, 0, scc8660_image_save[0],
                             SCC8660_W, SCC8660_H, SCC8660_W, SCC8660_H, 1);

    ips200_set_color(RGB565_GREEN, RGB565_WHITE);
    ips200_show_string(0, 120, "Slot:");
    ips200_show_uint(48, 120, g_current_slot, 1);
    ips200_show_string(58, 120, "Br:");
    ips200_show_uint(80, 120, g_camera_brightness, 3);
    ips200_set_color(RGB565_CYAN, RGB565_WHITE);
    ips200_show_string(95, 120, "FPS:");
    ips200_show_uint(127, 120, (uint32)(current_fps + 0.5f), 3);

    ips200_set_color(RGB565_BLUE, RGB565_WHITE);
    ips200_show_string(0, 136, "K1:");
    ips200_show_string(25, 136, "Lrn");
    ips200_show_string(55, 136, "K2:");
    ips200_show_string(80, 136, "Lrn");
    ips200_show_string(110, 136, "K3:");
    ips200_show_string(135, 136, "Trk");

    display_env_level();

    if (g_mode == MODE_LEARN) {
        display_learn_mode(scc8660_image_save);
    } else {
        display_track_mode(scc8660_image_save);
    }
}

#pragma section all restore
