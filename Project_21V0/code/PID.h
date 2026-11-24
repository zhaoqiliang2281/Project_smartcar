/*
 * PID.h
 *
 *  Created on: 2026年3月15日
 *      Author: 吉祥止止
 */

#include "zf_common_headfile.h"

#ifndef CODE_PID_H_
#define CODE_PID_H_

/*转向PID参数*/
#define Diversion_pid_KP    42.5f    // 比例系数
#define Diversion_pid_KI    0.0f    // 积分系数
#define Diversion_pid_KD    0.05f    // 微分系数

/*速度PID参数*/
#define Speed_pid_KP    6.0    // 比例系数
#define Speed_pid_KI    0.0    // 积分系数
#define Speed_pid_KD    0.0       // 微分系数

/*位置PID*/
#define Llocation_pid_KP    0.0   // 比例系数
#define Llocation_pid_KI    0.0    // 积分系数
#define Llocation_pid_KD    0.0    // 微分系数

/*角度PID*/
#define Angle_pid_KP    0.0   // 比例系数
#define Angle_pid_KI    0.0    // 积分系数
#define Angle_pid_KD    0.0    // 微分系数

/*左右PID*/
#define YSpeed_pid_KP    5.6   // 比例系数
#define YSpeed_pid_KI    0.0    // 积分系数
#define YSpeed_pid_KD    0.01    // 微分系数

/*距离PID*/
#define DSpeed_pid_KP    8.5   // 比例系数
#define DSpeed_pid_KI    0.0    // 积分系数
#define DSpeed_pid_KD    0.12    // 微分系数

typedef float fp32;

enum PID_MODE
{
    PID_POSITION = 0,
    PID_DELTA
};

typedef struct
{
    uint8_t mode;
    //PID 三参数
    fp32 Kp;
    fp32 Ki;
    fp32 Kd;

    fp32 max_out;  //最大输出
    fp32 max_iout; //最大积分输出

    fp32 set;
    fp32 fdb;

    fp32 out;
    fp32 Pout;
    fp32 Iout;
    fp32 Dout;
    fp32 Dbuf[3];  //微分项 0最新 1上一次 2上上次
    fp32 error[3]; //误差项 0最新 1上一次 2上上次

} pid_type_def;

typedef struct
{
    int32 Target;   //目标值
    int32 Actual;   //实际值
    int32 Result;   //结果值
    int32 Integral; //积分累计量

    uint8 Start;    //PID开始标志位

} pid_type_Parameter;


extern void PID_init(pid_type_def *pid, uint8_t mode, const fp32 PID[3], fp32 max_out, fp32 max_iout);
extern fp32 PID_calc(pid_type_def *pid, fp32 ref, fp32 set);
extern void PID_clear(pid_type_def *pid);

//PID参数声明
extern pid_type_Parameter Speed_PID;
extern pid_type_Parameter Location_PID;
extern pid_type_Parameter Diversion_PID;

extern pid_type_Parameter YDir_Speed_PID;
extern pid_type_Parameter DDir_Speed_PID;

//前轮转向电机目标值
extern int32 Target_Position;

void Dir_Motor_Encoder_PID_Update(void);

void All_PID_Init(void);
void PID_Controller_Update(void);


void Diversion_PID_Start(uint8 Start,int32 Actual,int32 Target);


void PID_DataShow(void);

void PID_adjust_parameter(void);
void All_PID_scan_adjust_parameter(void);


void KM3_All_PID_Init(void);
void KM3_PID_Controller_Update(void);
void YDir_Speed_PID_Start(uint8 Start,int32 Actual,int32 Target);
void DDir_Speed_PID_Start(uint8 Start,int32 Actual,int32 Target);

#endif /* CODE_PID_H_ */
