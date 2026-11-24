/*
 * @file    Color_discirmination.h
 * @brief   色块检测（HSL 阈值 + 连通域外接矩形）
 */
#ifndef CODE_COLOR_DISCIRMINATION_H_
#define CODE_COLOR_DISCIRMINATION_H_

#include "zf_common_headfile.h"

typedef struct {
    uint16 left, right, top, bottom;
    uint16 cx, cy;
    uint32 area;
    uint8 valid;
} color_blob_t;

// HSL 阈值范围：色相(Hue)、饱和度(Saturation)、亮度(Lightness)，均为 0~255
typedef struct {
    uint8 h_min, h_max;
    uint8 s_min, s_max;
    uint8 l_min, l_max;
} hsl_threshold_t;

extern color_blob_t g_blob;
extern hsl_threshold_t g_hsl_thresh;

void rgb565_to_rgb888(uint16 rgb, uint8 *r, uint8 *g, uint8 *b);
void rgb888_to_hsl(uint8 r, uint8 g, uint8 b, uint8 *h, uint8 *s, uint8 *l);
void sample_center_hsl(uint16 image[SCC8660_H][SCC8660_W]);
void find_color_blob(uint16 image[SCC8660_H][SCC8660_W], color_blob_t *blob);

#endif
