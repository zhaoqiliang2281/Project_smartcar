/*
 * PID.c
 *
 *  Created on: 2026年3月12日
 *      Author: 吉祥止止
 */

#include "zf_common_headfile.h"

PID_STEER pid_gps;  //pid

/************************************************
 *  函数名称: PID初始化：Servo_PID_Init(void)
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月26日
 *  备   注：
 ************************************************/
void GPS_PID_Init(double a,double b)
{
    pid_gps.kp = a;
    pid_gps.kd =b;

    pid_gps.steer_error_previous = 0;
    pid_gps.PID_result = 0;
}

/************************************************
 *  函数名称: 位置式PID：Loc_PID(void)
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年3月3日
 *  备   注：
 ************************************************/
double Loc_PID(double expect_Angle, double Actual_Angle, PID_STEER *pid)
{
    pid->steer_error = expect_Angle - Actual_Angle;
    pid->PID_result = pid->kp * pid->steer_error + pid->kd * (pid->steer_error - pid->steer_error_previous);
    // 保存误差
    pid->steer_error_previous = pid->steer_error;
    // 返回PID
    if(pid->PID_result < -900) {pid->PID_result =-900;}//角度为800-0-800
    if(pid->PID_result > 900){pid->PID_result = 900;}
    return pid->PID_result;
}

/************************************************
 *  函数名称: PID计算与前转向电机位置环赋值：void Front_Dir_PID_Control(double expect_Angle,double Actual_Angle, PID_STEER *pid)
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年3月16日
 *  备   注：
 ************************************************/
void Front_Dir_PID_Control(double expect_Angle,double Actual_Angle, PID_STEER *pid)
{
    Target_Position = -(int32)Loc_PID(expect_Angle,Actual_Angle, pid);    //赋给方向电机目标值
}

/************************************************
 *  函数名称: 计算两角最小角度差的调用与位置环PID计算与前转向电机赋值调用：Control_straight(void)
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年3月16日
 *  备   注：
 ************************************************/
double Error = 0;
void Control_straight(void)
{
    Error = angle_diff(azimuth,Yaw);    // 方向角与航向角之间的误差（用于控制）

    Front_Dir_PID_Control(0, Error, &pid_gps);

//    printf("%d,%d,%d\n\r",(int)next,(int)Error,(int)Target_Position);//上位机看值

}

void Dir_Control_straight(void)
{
    double Error = angle_diff(azimuth,Yaw);
    Target_Position = (int32)Loc_PID(0,Error, &pid_gps);

//    Target_Position = 0;
}


