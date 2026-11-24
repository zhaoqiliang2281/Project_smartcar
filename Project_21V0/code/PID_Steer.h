/*
 * PID.h
 *
 *  Created on: 2026年3月12日
 *      uthor: 吉祥止止
 */

#ifndef CODE_PID_STEER_H_
#define CODE_PID_STEER_H_

#include "zf_common_headfile.h"

extern double Error;    //外部声明用于测试

//PID_GPS结构体
typedef struct
{
    double kp;           //比例系数Kp
    double kd;           //微分系数Kd

    double steer_error;             // pid误差
    double steer_current;           // 当前角度
    double steer_error_previous;    // 上一次的误差
    double PID_result;
}PID_STEER;

extern PID_STEER pid_gps;//舵机pid
void GPS_PID_Init(double a,double b);

void Control_straight(void);

void Front_Dir_PID_Control(double expect_Angle,double Actual_Angle, PID_STEER *pid);

void Dir_Control_straight(void);

#endif /* CODE_PID_H_ */
