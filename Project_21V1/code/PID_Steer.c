/*
 * PID_Steer.c
 *
 *  Created on: 2026年3月12日
 *      Author: LiangZhaoqi
 */

#include "zf_common_headfile.h"

PID_STEER pid_gps;  //pid

/************************************************
 *  函数名称: GPS PID初始化
 *  函数说明:
 *  输入说明:
 *  输   出:
 *  返回值:
 *  更新时间: 2025年6月26日
 *  备   注:
 ************************************************/
void GPS_PID_Init(double a,double b)
{
    pid_gps.kp = a;
    pid_gps.kd = b;

    pid_gps.steer_error_previous = 0;
    pid_gps.PID_result = 0;
}

/************************************************
 *  函数名称: 位置式PID
 *  函数说明:
 *  输入说明:
 *  输   出:
 *  返回值:
 *  更新时间: 2026年3月3日
 *  备   注:
 ************************************************/
double Loc_PID(double expect_Angle, double Actual_Angle, PID_STEER *pid)
{
    pid->steer_error = expect_Angle - Actual_Angle;
    pid->PID_result = pid->kp * pid->steer_error + pid->kd * (pid->steer_error - pid->steer_error_previous);
    // 更新误差
    pid->steer_error_previous = pid->steer_error;
    // 限制PID输出
    if(pid->PID_result < -900) {pid->PID_result = -900;}
    if(pid->PID_result > 900)  {pid->PID_result = 900;}
    return pid->PID_result;
}

/************************************************
 *  函数名称: PID控制前轮转向位置闭环值
 *  函数说明:
 *  输入说明:
 *  输   出:
 *  返回值:
 *  更新时间: 2026年3月16日
 *  备   注:
 ************************************************/
void Front_Dir_PID_Control(double expect_Angle,double Actual_Angle, PID_STEER *pid)
{
    Angle_Target_Position = -(int32)Loc_PID(expect_Angle, Actual_Angle, pid);    // 将PID输出取反赋值给目标值
}

/************************************************
 *  函数名称: 计算当前航向与目标角度之差的值，通过PID控制前轮转向使车辆达到目标值
 *  函数说明:
 *  输入说明:
 *  输   出:
 *  返回值:
 *  更新时间: 2026年3月16日
 *  备   注:
 ************************************************/

double Error = 0;

void Control_straight(int8 direction)
{
    Error = angle_diff(azimuth, Yaw);    // 计算当前航向与目标角度之差（角度制）

    Angle_Target_Position = -GPS_IMU_PID_Start(Open, Error, 0, direction);

//    printf("%d,%d,%d\n\r",(int)next,(int)Error,(int)Angle_Target_Position);//位置式输出值
}
