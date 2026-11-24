#include "square_kalman_filter.h"

/* Tune these limits for the camera frame rate and target motion. */
#define SQUARE_KALMAN_PROCESS_NOISE       (4.0f)
#define SQUARE_KALMAN_MEASUREMENT_NOISE   (5.0f)

/* Jump rejection thresholds. Set to 0 to disable that channel. */
#define SQUARE_KALMAN_MAX_ANGLE_JUMP_DEG  (160.0f)
#define SQUARE_KALMAN_MAX_DISTANCE_JUMP_MM (1200u)

void square_kalman_init(square_kalman_filter_t *filter)
{
    memset(filter, 0, sizeof(square_kalman_filter_t));
    filter->angle_covariance = 25.0f;
    filter->distance_covariance = 2500.0f;
}

uint8 square_kalman_update(square_kalman_filter_t *filter,
                           int16 raw_angle_deg, uint32 raw_distance_mm,
                           int16 *filtered_angle_deg,
                           uint32 *filtered_distance_mm)
{
    float32 angle_measurement = (float32)raw_angle_deg;
    float32 distance_measurement = (float32)raw_distance_mm;
    float32 angle_gain;
    float32 distance_gain;
    float32 angle_jump;
    uint32 distance_jump;
    uint8 accept_angle;
    uint8 accept_distance;

    if((filter == NULL) || (filtered_angle_deg == NULL) ||
       (filtered_distance_mm == NULL))
        return 0u;

    if(!filter->initialized)
    {
        filter->angle = angle_measurement;
        filter->distance = distance_measurement;
        filter->initialized = 1u;
    }
    else
    {
        angle_jump = angle_measurement - filter->angle;
        if(angle_jump < 0.0f) angle_jump = -angle_jump;
        accept_angle = (SQUARE_KALMAN_MAX_ANGLE_JUMP_DEG == 0.0f) ||
                       (angle_jump <= SQUARE_KALMAN_MAX_ANGLE_JUMP_DEG);
        distance_jump = (raw_distance_mm > (uint32)filter->distance) ?
            (raw_distance_mm - (uint32)filter->distance) :
            ((uint32)filter->distance - raw_distance_mm);
        accept_distance = (SQUARE_KALMAN_MAX_DISTANCE_JUMP_MM == 0u) ||
                          (distance_jump <= SQUARE_KALMAN_MAX_DISTANCE_JUMP_MM);

        filter->angle_covariance += SQUARE_KALMAN_PROCESS_NOISE;
        angle_gain = filter->angle_covariance /
                     (filter->angle_covariance + SQUARE_KALMAN_MEASUREMENT_NOISE);
        if(accept_angle)
            filter->angle += angle_gain * (angle_measurement - filter->angle);
        filter->angle_covariance *= (1.0f - angle_gain);

        filter->distance_covariance += SQUARE_KALMAN_PROCESS_NOISE * 100.0f;
        distance_gain = filter->distance_covariance /
                        (filter->distance_covariance + 900.0f);
        if(accept_distance)
            filter->distance += distance_gain *
                               (distance_measurement - filter->distance);
        filter->distance_covariance *= (1.0f - distance_gain);
    }

    *filtered_angle_deg = (int16)(filter->angle >= 0.0f ?
                                  filter->angle + 0.5f : filter->angle - 0.5f);
    *filtered_distance_mm = (uint32)(filter->distance + 0.5f);
    return 1u;
}
