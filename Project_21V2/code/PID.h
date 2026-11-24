/*
 * PID.h
 *
 *  Created on: 2026年3月15日
 *      Author: LiangZhaoqi
 */

#include "zf_common_headfile.h"

#ifndef CODE_PID_H_
#define CODE_PID_H_

/*==============================PID参数====================================*/
/*GPS与IMU PID参数*/
#define GPS_IMU_PID_KP    16.8f   // 比例系数
#define GPS_IMU_PID_KI    0.68f    // 积分系数
#define GPS_IMU_PID_KD    0.0    // 微分系数

/*转向PID参数*/
#define Diversion_PID_KP    12.5f    // 比例系数
#define Diversion_PID_KI    0.1f    // 积分系数
#define Diversion_PID_KD    0.06f    // 微分系数

/*位置PID参数*/
#define Llocation_PID_KP    0.0   // 比例系数
#define Llocation_PID_KI    0.0    // 积分系数
#define Llocation_PID_KD    0.0    // 微分系数

/*角度PID参数*/
#define Angle_PID_KP    0.0   // 比例系数
#define Angle_PID_KI    0.0    // 积分系数
#define Angle_PID_KD    0.0    // 微分系数

/*左右PID参数*/
#define YSpeed_PID_KP    5.6   // 比例系数
#define YSpeed_PID_KI    0.0    // 积分系数
#define YSpeed_PID_KD    0.01    // 微分系数

/*距离PID参数*/
#define DSpeed_PID_KP    8.5   // 比例系数
#define DSpeed_PID_KI    0.0    // 积分系数
#define DSpeed_PID_KD    0.12    // 微分系数

typedef float fp32;

enum PID_MODE
{
    PID_POSITION = 0,
    PID_DELTA,
    PID_LOCATION
};

typedef enum
{
    Close = 0,
    Open = 1,
}PID_Start_enum;

typedef struct
{
    uint8_t mode;

    fp32 Kp;
    fp32 Ki;
    fp32 Kd;

    fp32 max_out;
    fp32 max_iout;

    fp32 set;
    fp32 fdb;

    fp32 out;
    fp32 Pout;
    fp32 Iout;
    fp32 Dout;

    fp32 Dbuf[3];

    fp32 error[3];

    int32 Result;
    int32 Target;
    int32 Actual;
    uint8 Start;

} pid_type_def;

// PID基础函数声明
void PID_init(pid_type_def *pid, uint8_t mode, const fp32 PID[3], fp32 max_out, fp32 max_iout);
fp32 PID_Update(pid_type_def *pid, fp32 ref, fp32 set);
void PID_clear(pid_type_def *pid);
void PID_Parameter_Set(pid_type_def *pid, float Kp,float Ki,float Kd);


//PID计算函数声明
void All_PID_Init(void);
void PID_Controller_Update(void);

// 后电机PID计算函数声明
void Motor_Speed_Encoder_PID_Start(PID_Start_enum Start, int32 Left_Target, int32 Right_Target);

//PID功能参数声明
int32 GPS_IMU_PID_Start(PID_Start_enum Start,int32 Actual,int32 Target);
int32 Diversion_PID_Start(PID_Start_enum Start,int32 Actual,int32 Target);
int32 Location_PID_Start(PID_Start_enum Start,int32 Actual,int32 Target);
int32 Angle_PID_Start(PID_Start_enum Start,int32 Actual,int32 Target);


#endif /* CODE_PID_H_ */
