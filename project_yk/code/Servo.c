/*
 * Servo.c
 *
 *  Created on: 2025年6月12日
 *      Author: 吉祥止止
 */

#include "zf_common_headfile.h"

/************************************************
 *  函数名称：舵机初始化：Steer_Init(void)
 *  功能说明：
 *  参数说明：
 *  参   数： 无
 *  函数返回：无
 *  修改时间：2025年6月12日
 *  备    注：
 ************************************************/
void Steer_Init(void)               //舵机初始化
{
    pwm_init(SERVO_MOTOR_PWM,SERVO_MOTOR_FREQ,(uint32)SERVO_MOTOR_DUTY(SERVO_MOTOR_MID));             //频率50~300
}

/************************************************
 *  函数名称：舵机角度设置：Steer_Set(uint32 Steer_duty)
 *  功能说明：
 *  参数说明：范围：75~95~115
 *  参   数：PWM占空比
 *  函数返回：无
 *  修改时间：2025年6月12日
 *  备    注：
 ************************************************/
void Steer_Set(uint32 Steer_duty)    //舵机角度
{
    //舵机限幅
    if(Steer_duty < SERVO_MOTOR_L_MAX){Steer_duty=SERVO_MOTOR_L_MAX;}   //75
    if(Steer_duty > SERVO_MOTOR_R_MAX){Steer_duty=SERVO_MOTOR_R_MAX;}   //115

    pwm_set_duty(SERVO_MOTOR_PWM,(uint32)SERVO_MOTOR_DUTY(Steer_duty)); //舵机PWM输出
}
