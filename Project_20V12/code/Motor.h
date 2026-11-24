/*
 * Motor.h
 *
 *  Created on: 2025年6月12日
 *      Author: LiangZhaoqi
 */

#ifndef CODE_MOTOR_H_
#define CODE_MOTOR_H_

extern int16 encoder[2];  //编码器读数存储变量

//无刷电机引脚定义
#define MAX_DUTY            (15)    //                                          // 最大占空比输出限制
#define DIR_CH1             (P02_6)                                            // 电机方向输出端口
#define PWM_CH1             (ATOM0_CH7_P02_7)                                  // PWM输出端口

//编码器引脚定义
#define ENCODER1_TIM        (TIM5_ENCODER)                                     // 编码器定时器
#define ENCODER1_PLUS       (TIM5_ENCODER_CH1_P10_3)                           // 编码器计数端口
#define ENCODER1_DIR        (TIM5_ENCODER_CH2_P10_1)                           // 编码器方向采值端口


void Motor_Init(void);
void Pwm_Motor(int32 speed_duty);


#endif /* CODE_MOTOR_H_ */
