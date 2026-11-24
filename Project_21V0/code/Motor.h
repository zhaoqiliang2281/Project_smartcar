/*
 * Motor.h
 *
 *  Created on: 2025年6月12日
 *      Author: 吉祥止止
 */

#ifndef CODE_MOTOR_H_
#define CODE_MOTOR_H_

#include "zf_common_headfile.h"


#define Mtor_USE_Zhufei (1)         //默认选逐飞
#define Dir_Mtor_USE_Zhufei (1)     //方向电机默认选逐飞

/*************宏共同选择区***************/
//底层电机最大输出PWM值
#define Dir_MOtor_Limit_Max_Out_Pwm    500      //转向电机限幅  -1000 ~ +1000
#define Back_MOtor_Limit_Max_Out_Pwm   800      //后驱电机限幅

#if Mtor_USE_Zhufei /***********后轮宏开始**************/
//逐飞驱动选择

//电机PWM通道引脚
#define Timer_PWMA              (ATOM0_CH4_P02_4)   //后电机PWM方向引脚
#define Back_Motor_DIR_PWMA              (P02_5)    //后电机方向引脚

#define Timer_PWMB              (ATOM0_CH6_P02_6)   //后电机PWM引脚
#define Back_Motor_DIR_PWMB              (P02_7)    //后电机方向引脚


#else
//科宇驱动选择

//电机PWM通道引脚
#define Timer_PWMA              (ATOM0_CH4_P02_4)
#define Timer_PWMB              (ATOM0_CH6_P02_6)
#define Timer_PWMC              (ATOM0_CH5_P02_5)
#define Timer_PWMD              (ATOM0_CH7_P02_7)


#endif  /****************后轮宏结束************/

#if Dir_Mtor_USE_Zhufei /***********前转向宏开始**************/

#define Timer_DIR_PWM           (ATOM0_CH1_P21_3)   //转向电机PWM方向引脚
#define Motor_DIR                        (P21_2)    //转向电机方向引脚

#else

#define Timer_DIR_PWMA          (ATOM0_CH1_P21_3)
#define Timer_DIR_PWMB          (ATOM0_CH0_P21_2)

#endif  /****************前转向宏结束************/

/************************下面模块共同区*******************/

void All_MOtor_init(void);
void Dir_Motor_SetPwm(int32 PWM);
void Back_Motor_SetPwm(int32 left_PWM,int32 right_PWM);

#endif /* CODE_MOTOR_H_ */
