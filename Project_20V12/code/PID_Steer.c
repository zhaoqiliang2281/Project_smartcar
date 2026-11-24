/*
 * PID.c
 *
 *  Created on: 2025年4月25日
 *      Author: LiangZhaoqi
 */

#include "zf_common_headfile.h"

PID_STEER pid_gps;//舵机pid

/************************************************
 *  函数名称: 舵机PID初始化：Servo_PID_Init(void)
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月26日
 *  备   注：
 ************************************************/
void Servo_PID_Init(double a,double b)
{
    pid_gps.kp = a;
    pid_gps.kd =b;
    pid_gps.steer_error_previous = 0;
    pid_gps.PID_result = 0;
}

/************************************************
 *  函数名称: 舵机位置式PID：Loc_PID(void)
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月26日
 *  备   注：
 ************************************************/
double Loc_PID(double expect_Angle, double Actual_Angle, PID_STEER *pid)
{
    pid->steer_error = expect_Angle - Actual_Angle;
    pid->PID_result = pid->kp * pid->steer_error + pid->kd * (pid->steer_error - pid->steer_error_previous);
    // 保存误差
    pid->steer_error_previous = pid->steer_error;
    // 返回PID舵机值
    if(pid->PID_result < -20) {pid->PID_result =-20;}//舵机角度为75-95-115，差值为20，限幅20，
    if(pid->PID_result > 20){pid->PID_result = 20;}
    return pid->PID_result;
}

/************************************************
 *  函数名称: 舵机PID计算与舵机赋值：Steer_PID_Control(void)
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月26日
 *  备   注：
 ************************************************/
uint32 steer_duty = 0;//舵机PID输出占空比
void Steer_PID_Control(double expect_Angle,double Actual_Angle, PID_STEER *pid)
{
    steer_duty = SERVO_MOTOR_MID + (int32)Loc_PID(expect_Angle,Actual_Angle, pid);  //计算出PID，得出结果加上机械中值赋给舵机占空比
    Steer_Set(steer_duty);  //赋给舵机
}

/************************************************
 *  函数名称: 计算两角最小角度差的调用与舵机PID计算与舵机赋值调用：Control_straight(void)
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月26日
 *  备   注：
 ************************************************/
double Error = 0; // 方向角与航向角之间的误差（用于控制）
void Control_straight(void)
{
    Error = angle_diff(azimuth,Yaw);
    Steer_PID_Control(0, Error, &pid_gps);
}

