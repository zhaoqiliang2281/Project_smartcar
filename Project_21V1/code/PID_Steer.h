/*
 * PID_Steer.h
 *
 *  Created on: 2026年3月12日
 *      Author: LiangZhaoqi
 */

#ifndef CODE_PID_STEER_H_
#define CODE_PID_STEER_H_

#include "zf_common_headfile.h"

//PID_GPS结构体
typedef struct
{
    double kp;           // 比例系数Kp
    double kd;           // 微分系数Kd

    double steer_error;             // pid误差
    double steer_current;           // 当前角度值
    double steer_error_previous;    // 上一帧的误差
    double PID_result;
}PID_STEER;

extern double Error;    // 方向偏差值（可调参数）

extern PID_STEER pid_gps;

void GPS_PID_Init(double a,double b);

void Control_straight(int8 direction);

void Front_Dir_PID_Control(double expect_Angle,double Actual_Angle, PID_STEER *pid);

#endif /* CODE_PID_H_ */
