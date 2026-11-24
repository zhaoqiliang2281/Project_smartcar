/*
 * @file    Color_discirmination.c
 * @brief   基于 HSL 的色块检测（SCC8660 160x120 RGB565）
 *
 * 思路：
 * 1) RGB565 -> RGB888 -> HSL（使用 H/S/L 分离降低光照变化影响）
 * 2) 以“画面中心附近的目标色像素”作为种子点
 * 3) 从种子点做 4 邻域连通域搜索，统计外接矩形与面积
 */
#include "Color_discirmination.h"

color_blob_t g_blob;
// H/S/L 阈值（0~255），默认给一个偏“黄”的范围，运行中可按键重新采样中心点生成阈值
hsl_threshold_t g_hsl_thresh = {17, 57, 100, 255, 50, 255};

uint8 g_swap_rgb565 = 1;   // 如果采样到的 RGB 明显偏色，可置 1 进行 RGB565 高低字节交换
// 以中心点 HSL 为基准扩展阈值（可根据现场光照调整）
#define HSL_H_DELTA   20
#define HSL_S_DELTA   30
#define HSL_L_DELTA   40

static inline uint8 h_in_range_wrap(uint8 h, uint8 h_min, uint8 h_max)
{
    // Hue 是环形空间（0~255）：当 h_min > h_max 表示区间跨越 0
    if (h_min <= h_max) return (h >= h_min && h <= h_max);
    else                return (h >= h_min || h <= h_max);
}

static inline uint8 hsl_match(uint8 h, uint8 s, uint8 l)
{
    return h_in_range_wrap(h, g_hsl_thresh.h_min, g_hsl_thresh.h_max) &&
           (s >= g_hsl_thresh.s_min && s <= g_hsl_thresh.s_max) &&
           (l >= g_hsl_thresh.l_min && l <= g_hsl_thresh.l_max);
}

/*
 * 内存说明：
 * 直接使用 visited[H][W] + 全尺寸队列会占用较多 DSRAM。
 * 这里用 1bit/像素的 visited 位图 + 有界队列，兼顾稳定性与 RAM 占用。
 */
#define SCC8660_PIXELS   ((uint16)(SCC8660_W * SCC8660_H))
#define VISITED_BYTES    (((uint32)SCC8660_PIXELS + 7u) / 8u)
#define BFS_QUEUE_CAP    (16384u)

static inline void visited_clear(uint8 *bits)
{
    memset(bits, 0, (size_t)VISITED_BYTES);
}

static inline void visited_set(uint8 *bits, uint16 idx)
{
    bits[(uint32)idx >> 3u] |= (uint8)(1u << (idx & 7u));
}

static inline uint8 visited_get(const uint8 *bits, uint16 idx)
{
    return (uint8)(bits[(uint32)idx >> 3u] & (uint8)(1u << (idx & 7u)));
}

// 仅当邻居像素满足 HSL 阈值时才入队并置位，避免无效点占满队列
static inline void bfs_try_push(const uint16 image[SCC8660_H][SCC8660_W], uint8 *bits,
                                uint16 idx, uint16 *bfs_q, uint32 *qt, uint8 *q_overflow)
{
    if (visited_get(bits, idx)) {
        return;
    }
    uint16 px = (uint16)(idx % (uint16)SCC8660_W);
    uint16 py = (uint16)(idx / (uint16)SCC8660_W);

    uint16 rgb = image[py][px];
    uint8 r, g, b;
    rgb565_to_rgb888(rgb, &r, &g, &b);
    uint8 h, s, l;
    rgb888_to_hsl(r, g, b, &h, &s, &l);
    if (!hsl_match(h, s, l)) {
        return;
    }

    visited_set(bits, idx);
    if (*qt < BFS_QUEUE_CAP) {
        bfs_q[*qt] = idx;
        (*qt)++;
    } else {
        *q_overflow = 1;
    }
}

// RGB565 -> RGB888（若需要可做高低字节交换）
void rgb565_to_rgb888(uint16 rgb, uint8 *r, uint8 *g, uint8 *b) {
    if (g_swap_rgb565) {
            rgb = (rgb >> 8) | (rgb << 8);   // 高低字节交换
    }
    uint8 r5 = (rgb >> 11) & 0x1F;
    uint8 g6 = (rgb >> 5)  & 0x3F;
    uint8 b5 = rgb & 0x1F;
    *r = (r5 << 3) | (r5 >> 2);
    *g = (g6 << 2) | (g6 >> 4);
    *b = (b5 << 3) | (b5 >> 2);
}

// RGB888 -> HSL（输出范围 0~255，便于做阈值判断）
void rgb888_to_hsl(uint8 r, uint8 g, uint8 b, uint8 *h, uint8 *s, uint8 *l) {
    uint8 max = r > g ? (r > b ? r : b) : (g > b ? g : b);
    uint8 min = r < g ? (r < b ? r : b) : (g < b ? g : b);
    uint16 delta = max - min;
    uint16 sum = max + min;

    // L = (max + min) / 2
    *l = (uint8)(sum / 2);

    if (delta == 0) {
        *h = 0;
        *s = 0;
        return;
    }

    // S 根据亮度做分段归一化
    if (*l < 128) {
        *s = (uint8)((delta * 255) / sum);
    } else {
        *s = (uint8)((delta * 255) / (510 - sum));
    }

    // H (0~360) 映射到 0~255
    int16 hue;
    if (max == r) {
        hue = (int16)(60 * ((int16)(g - b)) / (int16)delta);
    } else if (max == g) {
        hue = (int16)(120 + 60 * ((int16)(b - r)) / (int16)delta);
    } else {
        hue = (int16)(240 + 60 * ((int16)(r - g)) / (int16)delta);
    }
    if (hue < 0) hue += 360;
    *h = (uint8)((hue * 255) / 360);
}

// 读取图像中心点，生成 H/S/L 阈值范围
void sample_center_hsl(uint16 image[SCC8660_H][SCC8660_W]) {
    uint16 rgb = image[SCC8660_H/2][SCC8660_W/2];
    uint8 r_t, g_t, b_t;
    rgb565_to_rgb888(rgb, &r_t, &g_t, &b_t);

    uint8 h_t, s_t, l_t;
    rgb888_to_hsl(r_t, g_t, b_t, &h_t, &s_t, &l_t);

    // Hue 使用环绕阈值（允许跨越 0/255）
    g_hsl_thresh.h_min = (uint8)(h_t - HSL_H_DELTA);
    g_hsl_thresh.h_max = (uint8)(h_t + HSL_H_DELTA);
    g_hsl_thresh.s_min = (s_t > HSL_S_DELTA) ? (s_t - HSL_S_DELTA) : 0;
    g_hsl_thresh.s_max = (s_t + HSL_S_DELTA) < 255 ? (s_t + HSL_S_DELTA) : 255;
    g_hsl_thresh.l_min = (l_t > HSL_L_DELTA) ? (l_t - HSL_L_DELTA) : 0;
    g_hsl_thresh.l_max = (l_t + HSL_L_DELTA) < 255 ? (l_t + HSL_L_DELTA) : 255;
}

// 色块检测：中心附近找种子点 + 4 邻域连通域搜索，输出外接矩形与面积
void find_color_blob(uint16 image[SCC8660_H][SCC8660_W], color_blob_t *blob) {
    uint16 min_x = SCC8660_W, max_x = 0;
    uint16 min_y = SCC8660_H, max_y = 0;
    uint32 count = 0;

    // 1) 选取“最靠近画面中心”的目标色像素作为种子点（比从左上角遇到的第一个点更稳定）
    int16 seed_x = -1, seed_y = -1;
    uint32 best_d2 = 0xFFFFFFFFu;
    const int16 cx0 = (int16)(SCC8660_W / 2);
    const int16 cy0 = (int16)(SCC8660_H / 2);

    for (uint16 y = 0; y < SCC8660_H; y++) {
        for (uint16 x = 0; x < SCC8660_W; x++) {
            uint16 rgb = image[y][x];
            uint8 r, g, b;
            rgb565_to_rgb888(rgb, &r, &g, &b);
            uint8 h, s, l;
            rgb888_to_hsl(r, g, b, &h, &s, &l);
            if (hsl_match(h, s, l)) {
                int16 dx = (int16)x - cx0;
                int16 dy = (int16)y - cy0;
                uint32 d2 = (uint32)(dx * dx + dy * dy);
                if (d2 < best_d2) {
                    best_d2 = d2;
                    seed_x = (int16)x;
                    seed_y = (int16)y;
                }
            }
        }
    }

    if (seed_x < 0) {
        blob->valid = 0;
        return;
    }

    static uint8 visited_bits[VISITED_BYTES];
    static uint16 bfs_q[BFS_QUEUE_CAP];

    visited_clear(visited_bits);

    uint16 seed_idx = (uint16)((uint16)seed_y * (uint16)SCC8660_W + (uint16)seed_x);
    uint32 qh = 0, qt = 0;
    uint8 q_overflow = 0;

    visited_set(visited_bits, seed_idx);
    bfs_q[qt++] = seed_idx;

    while (qh < qt && !q_overflow) {
        uint16 idx = bfs_q[qh++];
        uint16 px = (uint16)(idx % (uint16)SCC8660_W);
        uint16 py = (uint16)(idx / (uint16)SCC8660_W);

        uint16 rgb = image[py][px];
        uint8 r, g, b;
        rgb565_to_rgb888(rgb, &r, &g, &b);
        uint8 h, s, l;
        rgb888_to_hsl(r, g, b, &h, &s, &l);
        if (!hsl_match(h, s, l)) {
            continue;
        }

        if (px < min_x) min_x = px;
        if (px > max_x) max_x = px;
        if (py < min_y) min_y = py;
        if (py > max_y) max_y = py;
        count++;

        if (px > 0) {
            bfs_try_push(image, visited_bits, (uint16)(idx - 1u), bfs_q, &qt, &q_overflow);
        }
        if ((uint16)(px + 1u) < (uint16)SCC8660_W) {
            bfs_try_push(image, visited_bits, (uint16)(idx + 1u), bfs_q, &qt, &q_overflow);
        }
        if (py > 0) {
            bfs_try_push(image, visited_bits, (uint16)(idx - (uint16)SCC8660_W), bfs_q, &qt, &q_overflow);
        }
        if ((uint16)(py + 1u) < (uint16)SCC8660_H) {
            bfs_try_push(image, visited_bits, (uint16)(idx + (uint16)SCC8660_W), bfs_q, &qt, &q_overflow);
        }

        if (count > (uint32)(SCC8660_W * SCC8660_H / 2)) {
            break;
        }
    }

    if (q_overflow) {
        blob->valid = 0;
        return;
    }

    if (count > 30) {
        blob->valid  = 1;
        blob->left   = min_x;
        blob->right  = max_x;
        blob->top    = min_y;
        blob->bottom = max_y;
        blob->area   = count;
        blob->cx     = (min_x + max_x) / 2;
        blob->cy     = (min_y + max_y) / 2;
    } else {
        blob->valid = 0;
    }
}
