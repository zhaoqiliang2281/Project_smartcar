/*
 * @file    Color_discirmination.h
 * @brief   HSV 距离图 + 十字扩展膨胀 目标识别
 *
 * 算法流程：
 * 1. RGB565 -> HSV (纯整数)
 * 2. 计算与目标 HSV 的加权距离 -> 相似度图
 * 3. 找最亮点作为种子
 * 4. 十字扩展 + 膨胀迭代 -> 精确中心坐标
 *
 * 特点：软匹配替代硬阈值，光照鲁棒性更强
 */
#ifndef CODE_COLOR_DISCIRMINATION_H_
#define CODE_COLOR_DISCIRMINATION_H_

#include "zf_common_headfile.h"

#ifndef SCC8660_W
#define SCC8660_W   (160)
#endif
#ifndef SCC8660_H
#define SCC8660_H   (120)
#endif

// ---- HSV 颜色 ----
typedef struct {
    uint8 h, s, v;       // H:0-255(0-360deg), S:0-255, V:0-255
} hsv_color_t;

// ---- 检测结果 ----
typedef struct {
    uint16 cx, cy;        // 中心坐标
    uint16 left, right, top, bottom;
    uint32 area;
    uint8  valid;
} color_blob_t;

// ---- HSV 阈值(兼容旧类型名) ----
typedef struct {
    uint8 a_min, a_max;
    uint8 b_min, b_max;
    uint8 l_min, l_max;
} lab_threshold_t;

typedef struct { uint8 a, b, l; } lab_sample_t;
#define SAMPLE_COUNT_MAX  (9)

typedef struct {
    lab_threshold_t thresh;
    uint8  a_median, b_median, l_median;
    uint8  a_mad, b_mad, l_mad;
    uint16 avg_brightness;
} color_learn_result_t;

// ---- 环境光等级 ----
typedef enum {
    ENV_NORMAL = 0, ENV_BRIGHT = 1, ENV_SHADOW = 2, ENV_OVERCAST = 3,
} env_light_level_t;

typedef enum { MODE_LEARN = 0, MODE_TRACK = 1 } work_mode_t;

// ---- 全局变量 ----
extern color_blob_t         g_blob;
extern lab_threshold_t      g_lab_thresh;
extern color_learn_result_t g_learn_result;
extern env_light_level_t    g_env_level;
extern uint8                g_swap_rgb565;

// ---- 颜色转换 ----
void rgb565_to_rgb888(uint16 rgb, uint8 *r, uint8 *g, uint8 *b);
void rgb888_to_lab(uint8 r, uint8 g, uint8 b, uint8 *la, uint8 *aa, uint8 *bb);

// ---- 学习 ----
void sample_multi_point_lab(const uint16 image[SCC8660_H][SCC8660_W]);
void sample_region_lab(const uint16 image[SCC8660_H][SCC8660_W],
                       uint16 cx, uint16 cy, uint16 half_w, uint16 half_h);

uint16 estimate_global_brightness(const uint16 image[SCC8660_H][SCC8660_W]);
env_light_level_t detect_env_light(const uint16 image[SCC8660_H][SCC8660_W]);
void adapt_threshold_to_env(env_light_level_t env, lab_threshold_t *thresh,
                            const color_learn_result_t *learn);

void find_color_blob(const uint16 image[SCC8660_H][SCC8660_W], color_blob_t *blob);
void find_color_blob_fast(const uint16 image[SCC8660_H][SCC8660_W],
                          color_blob_t *blob, const color_blob_t *prev_blob);

void vision_init(void);
void vision_process_frame(void);

#endif
