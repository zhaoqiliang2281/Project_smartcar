/*
 * PID.h
 *
 *  Created on: 2025年4月25日
 *      Author: LiangZhaoqi
 */

#ifndef CODE_PID_STEER_H_
#define CODE_PID_STEER_H_

#include "zf_common_headfile.h"

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

void Servo_PID_Init(double a,double b);

void Control_straight(void);

#endif /* CODE_PID_H_ */
