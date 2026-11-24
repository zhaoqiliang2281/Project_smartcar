/*
 * user_display_gps.h
 *
 *  Created on: 2025年6月16日
 *      Author: liang2281
 */

#ifndef CODE_USER_DISPLAY_GPS_H_
#define CODE_USER_DISPLAY_GPS_H_

#include "zf_common_headfile.h"

#define EARTH_RADIUS1   (6371000.0)  // 地球平均半径，单位为米

#define EPSILON    (1e-9)            // 精度，最小数值

#define USER_PI    (3.1415926535898)

#define DISPLAY_POINT_MAX       (200)        // 最大点位数量，可根据需要修改

typedef struct
{
    double lat;  // 纬度
    double lon;  // 经度
} gps_point;

typedef struct
{
    double x;   // 平面坐标系X坐标（单位为米，以东向为正）
    double y;   // 平面坐标系Y坐标（单位为米，以北向为正）
} plane_point;

typedef struct
{
    double x;   // 屏幕X坐标
    double y;   // 屏幕Y坐标
} screen_point;

typedef enum
{
    SCREEN_IPS114,
    SCREEN_IPS200_SPI,
    SCREEN_IPS200_PARALLEL8,
    SCREEN_TFT180,
} screen_type_enum;

extern gps_point gps_point_test[];

void user_gps_transition(gps_point *gps_point_input, int16 point_num);          // GPS坐标系转换为屏幕显示位置
void user_gps_display(uint16 display_color);                                   // 屏幕显示路径点
void user_gps_display_init(screen_type_enum screen_type, int16 start_x, int16 start_y, int16 width, int16 high);
void user_gps_get_screen_point(int16 index, uint16 *x, uint16 *y);              // 获取指定索引点的屏幕坐标（包含显示偏移）
int16 user_gps_get_point_count(void);                                           // 获取转换后的点数量
void ips200_draw_circle(uint16 cx, uint16 cy, uint16 r, const uint16 color);   // IPS200 画圆函数
void Function(void);

#endif /* CODE_USER_DISPLAY_GPS_H_ */
