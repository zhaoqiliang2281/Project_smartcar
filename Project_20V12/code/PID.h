/*
 * PID.h
 *
 *  Created on: 2025年6月15日
 *      Author: LiangZhaoqi
 */

#include "zf_common_headfile.h"

#ifndef CODE_PID_H_
#define CODE_PID_H_

//学习板PID参数

/*速度PID*/
//#define Speed_pid_KP    0.34    // 比例系数
//#define Speed_pid_KI    0.053    // 积分系数
//#define Speed_pid_KD    0.1       // 微分系数

/*位置PID*/
//#define Llocation_pid_KP    -9.34   // 比例系数
//#define Llocation_pid_KI    -0.0    // 积分系数
//#define Llocation_pid_KD    -0.0    // 微分系数

//自画板PID参数
/*速度PID*/
#define Speed_pid_KP    0.40    // 比例系数
#define Speed_pid_KI    0.00    // 积分系数
#define Speed_pid_KD    0.11       // 微分系数

/*位置PID*/
//#define Llocation_pid_KP    -20.64   // 比例系数
//#define Llocation_pid_KI    -0.0    // 积分系数
//#define Llocation_pid_KD    -0.0    // 微分系数

#define Llocation_pid_KP    -20.64   // 比例系数
#define Llocation_pid_KI    -0.0    // 积分系数
#define Llocation_pid_KD    -0.0    // 微分系数

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

extern void PID_init(pid_type_def *pid, uint8_t mode, const fp32 PID[3], fp32 max_out, fp32 max_iout);
extern fp32 PID_calc(pid_type_def *pid, fp32 ref, fp32 set);
extern void PID_clear(pid_type_def *pid);

extern int16 resultSpeed;   //速度PID计算结果
extern int16 resultServo;  //舵机PID计算结果

void All_PID_Init(void);
void All_PID_scan(void);

void PID_DataShow(void);

void PID_adjust_parameter(void);
void All_PID_scan_adjust_parameter(void);

void Speed_PID_Init(int8 num);
void Speed_Location_Init(int16 num);
void servo_PID_Init(float Servo_pid_KP,float Servo_pid_KI,float Servo_pid_KD);

#endif /* CODE_PID_H_ */
