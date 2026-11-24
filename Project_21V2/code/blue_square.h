#ifndef BLUE_SQUARE_H
#define BLUE_SQUARE_H

#include "zf_common_headfile.h"

/* Select the filled-circle target color here.  Legacy type/function names
 * are retained so the application and communication interfaces stay the
 * same. */
#define CIRCLE_COLOR_PURPLE            (0u)
#define CIRCLE_COLOR_BLUE              (1u)
//#define TARGET_CIRCLE_COLOR            (CIRCLE_COLOR_PURPLE)
#define TARGET_CIRCLE_COLOR            (CIRCLE_COLOR_PURPLE)

/* Integer HSV thresholds. Hue is in degrees [0, 359], saturation and value
 * are [0, 255]. Tune these limits if the target or lighting changes. */
#define PURPLE_H_MIN                   (255u)
#define PURPLE_H_MAX                   (330u)
#define PURPLE_S_MIN                   (70u)
#define PURPLE_V_MIN                   (25u)

#define BLUE_H_MIN                     (190u)
#define BLUE_H_MAX                     (250u)
/* 收紧饱和度/明度下限（原 S_MIN=70、V_MIN=25）：
   目标是"低饱和蓝"，提高 S/V 可把背景里的灰蓝、暗蓝过滤掉，减少误识别。
   若仍误识别其它蓝色可继续调高；若目标识别不到则调低。 */
#define BLUE_S_MIN                     (100u)
#define BLUE_V_MIN                     (80u)

/* One-key colour sampling ("lock") tuning.  SAMPLE_REGION_HALF is the half
   width of the centred square region sampled, i.e. the region is
   (2*SAMPLE_REGION_HALF+1) x (2*SAMPLE_REGION_HALF+1) pixels.  Margins widen
   the captured range so the target still matches under slightly different
   lighting. */
#define SAMPLE_REGION_HALF             (4u)    /* 9x9 pixel region */
#define SAMPLE_MARGIN_H                (12u)   /* hue margin, degrees */
#define SAMPLE_MARGIN_S                (20u)   /* saturation margin */
#define SAMPLE_MARGIN_V                (20u)   /* value margin */

/* Runtime-adjustable colour threshold. */
typedef struct
{
    uint16 h_min;
    uint16 h_max;
    uint16 s_min;
    uint16 v_min;
} color_threshold_t;

typedef struct
{
    uint8  found;
    uint16 x_min;
    uint16 y_min;
    uint16 x_max;
    uint16 y_max;
    uint16 center_x;
    uint16 center_y;
    uint16 area;
    uint8  score;
    /* Signed horizontal image offset in pixels: left is negative, right is
       positive, relative to the frame centre line. */
    int16  angle_deg;
    uint16 vertical_length_px;
    uint32 distance_mm;
    uint16 tl_x;
    uint16 tl_y;
    uint16 tr_x;
    uint16 tr_y;
    uint16 br_x;
    uint16 br_y;
    uint16 bl_x;
    uint16 bl_y;
} blue_square_result_t;

/* Distance calibration for the selected circle.  Distance is estimated from
 * the vertical image diameter (distance = real diameter * focal length /
 * vertical pixel diameter). */
#define PURPLE_CIRCLE_REAL_DIAMETER_MM       (150u)
#define PURPLE_CIRCLE_FOCAL_LENGTH_PX        (70u)

void blue_square_detect(const uint16 *image, uint16 width, uint16 height,
                        blue_square_result_t *result);
void blue_square_show_mask(uint16 width, uint16 height);
void blue_square_draw(const blue_square_result_t *result);

void blue_square_set_threshold(const color_threshold_t *thr);
void blue_square_get_threshold(color_threshold_t *thr);
uint8 blue_square_sample_threshold(const uint16 *image, uint16 width,
                                   uint16 height, color_threshold_t *thr);

/* Camera tuning application: threshold/exposure adjustment loop.
 * Encapsulates the demo previously living in user/cpu0_main.c so that the
 * main file only drives the top-level loop. */
void blue_square_camera_init(void);
void blue_square_camera_process(void);

extern uint8 g_debug_mode;

#define LOST_TIMEOUT  (30)   /* 15 → 30，约 0.5 秒后才停车 */

#endif
