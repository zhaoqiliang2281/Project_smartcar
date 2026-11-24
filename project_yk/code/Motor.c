/*
 * Motor.c
 *
 *  Created on: 2025年6月12日
 *      Author: 吉祥止止
 */

#include "zf_common_headfile.h"

//编码器接收变量
int16 encoder[2] = {0};  //编码器读数存储变量,范围：-2147483648到2147483647  //32749

/************************************************
 *  函数名称：电机初始化：Motor_Init(void)
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月12日
 *  备    注：
 ************************************************/
void Motor_Init(void)
{
    pwm_init(PWM_CH1, 3450, 0);                                        // PWM 通道1 初始化频率3.45KHz 占空比初始为0

    gpio_init(DIR_CH1, GPO, GPIO_HIGH, GPO_PUSH_PULL);                 // 初始化电机方向输出引脚

    encoder_dir_init(ENCODER1_TIM, ENCODER1_PLUS, ENCODER1_DIR);       // 初始化编码器采值引脚及定时器
}


/************************************************
 *  函数名称：电机速度设置：Pwm_Motor(int32 speed_duty)
 *  功能说明：
 *  参数说明：范围：-50~+50
 *  参   数：PWM占空比
 *  函数返回：无
 *  修改时间：2025年6月12日
 *  备    注：
 ************************************************/
void Pwm_Motor(int32 speed_duty)
{
    //电机占空比赋值
    if(speed_duty >= 0)                                                               // 正转
    {
        pwm_set_duty(PWM_CH1, speed_duty * (PWM_DUTY_MAX / 100));                     // 计算占空比
        gpio_set_level(DIR_CH1, 1);//设置正转
    }
    else                                                                        // 反转
    {
        pwm_set_duty(PWM_CH1, -speed_duty * (PWM_DUTY_MAX / 100));
        gpio_set_level(DIR_CH1, 0);//设置反反正转
    }
}

