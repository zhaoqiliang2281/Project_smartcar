/*
 * Motor.h
 *
 *  Created on: 2025年6月12日
 *      Author: LiangZhaoqi
 */

#ifndef CODE_MOTOR_H_
#define CODE_MOTOR_H_

#include "zf_common_headfile.h"

#define Motor_USE_Zhufei (1)         //后驱电机默认选逐飞
#define Dir_Motor_USE_Zhufei (1)     //方向电机默认选逐飞

//底层电机最大输出PWM值
#define Dir_MOtor_Limit_Max_Out_Pwm    500      //转向电机限幅  -1000 ~ +1000
#define Back_MOtor_Limit_Max_Out_Pwm   800      //后驱电机限幅

/*右速度PID参数*/
#define Right_Speed_Pid_KP    4.29f     // 比例系数
#define Right_Speed_Pid_KI    0.455f    // 积分系数
#define Right_Speed_Pid_KD    0.085f    // 微分系数

/*左速度PID参数*/
#define Left_Speed_Pid_KP    4.29f      // 比例系数
#define Left_Speed_Pid_KI    0.455f     // 积分系数
#define Left_Speed_Pid_KD    0.085f     // 微分系数

#if Motor_USE_Zhufei    //逐飞驱动选择

#define Timer_PWMA              (ATOM0_CH4_P02_4)   //后电机PWM方向引脚
#define Back_Motor_DIR_PWMA              (P02_5)    //后电机方向引脚

#define Timer_PWMB              (ATOM0_CH6_P02_6)   //后电机PWM引脚
#define Back_Motor_DIR_PWMB              (P02_7)    //后电机方向引脚


#else   //科宇驱动选择

#define Timer_PWMA              (ATOM0_CH4_P02_4)
#define Timer_PWMB              (ATOM0_CH6_P02_6)
#define Timer_PWMC              (ATOM0_CH5_P02_5)
#define Timer_PWMD              (ATOM0_CH7_P02_7)


#endif

#if Dir_Motor_USE_Zhufei

#define Timer_DIR_PWM           (ATOM0_CH0_P21_2)   //转向电机PWM方向引脚     2026年6月30号新板子引脚
#define Motor_DIR                        (P21_3)    //转向电机方向引脚

#else

#define Timer_DIR_PWMA          (ATOM0_CH1_P21_3)
#define Timer_DIR_PWMB          (ATOM0_CH0_P21_2)

#endif

extern pid_type_def PID_Right_Speed_Encoder;
extern pid_type_def PID_Left_Speed_Encoder;

void All_MOtor_init(void);
void Dir_Motor_SetPwm(int32 PWM);
void Back_Motor_SetPwm(int32 left_PWM,int32 right_PWM);

#endif /* CODE_MOTOR_H_ */
