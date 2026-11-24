#ifndef SQUARE_KALMAN_FILTER_H
#define SQUARE_KALMAN_FILTER_H

#include "zf_common_headfile.h"
#include "blue_square.h"

typedef struct
{
    float32 angle;
    float32 distance;
    float32 angle_covariance;
    float32 distance_covariance;
    uint8 initialized;
} square_kalman_filter_t;

void square_kalman_init(square_kalman_filter_t *filter);
uint8 square_kalman_update(square_kalman_filter_t *filter,
                           int16 raw_angle_deg, uint32 raw_distance_mm,
                           int16 *filtered_angle_deg,
                           uint32 *filtered_distance_mm);

#endif
