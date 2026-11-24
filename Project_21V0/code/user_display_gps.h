/*
 * user_display_gps.h
 *
 *  Created on: 2025年6月16日
 *      Author: 吉祥止止
 */

#ifndef CODE_USER_DISPLAY_GPS_H_
#define CODE_USER_DISPLAY_GPS_H_

#include "zf_common_headfile.h"

#define EARTH_RADIUS1   (6371000.0)  //地球平均半径（单位：米）

#define EPSILON    (1e-9)           //浮点  最小误差

#define USER_PI    (3.1415926535898)

#define DISPLAY_POINT_MAX       (40)        //最大点位数量  可自行修改

typedef struct
{
        double lat;  //纬度
        double lon;  //经度
}gps_point;

typedef struct
{
        double x;  //东方向坐标（单位：米，相对于第一个点）
        double y;   //北方向坐标（单位：米，相对于第一个点）
}plane_point;

typedef struct
{
        double x;   //屏幕X坐标
        double y;   //屏幕Y坐标
}screen_point;
typedef enum
{
    SCREEN_IPS114,
    SCREEN_IPS200_SPI,
    SCREEN_IPS200_PARALLEL8,
    SCREEN_TFT180,
}screen_type_enum;

extern gps_point gps_point_test[];

void user_gps_transition(gps_point *gps_point_input,int16 point_num);   //GPS坐标系  转化为  屏幕显示位置
void user_gps_display (uint16 display_color);                                   //屏幕显示输出函数
void user_gps_display_init (screen_type_enum screen_type,int16 start_x,int16 start_y,int16 width,int16 high);
void Function(void);



#endif /* CODE_USER_DISPLAY_GPS_H_ */
