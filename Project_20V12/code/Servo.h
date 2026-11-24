/*
 * Servo.h
 *
 *  Created on: 2025年6月12日
 *      Author: LiangZhaoqi
 */

#ifndef CODE_SERVO_H_
#define CODE_SERVO_H_

//舵机引脚定义
#define SERVO_MOTOR_PWM             (ATOM1_CH1_P33_9)               // 定义主板上舵机对应引脚
#define SERVO_MOTOR_FREQ            (100)                           // 定义主板上舵机频率  请务必注意范围 50-300

#define SERVO_MOTOR_MID             (90)                            // 定义主板上舵机活动范围 角度
#define SERVO_MOTOR_L_MAX           (70)                            // 中间角度
#define SERVO_MOTOR_R_MAX           (110)                           // 定义主板上舵机活动范围 角度

// ------------------ 舵机占空比计算方式 ------------------
// 舵机对应的 0-180 活动角度对应 控制脉冲的 0.5ms-2.5ms 高电平

// 那么不同频率下的占空比计算方式就是
// PWM_DUTY_MAX/(1000/freq)*(1+Angle/180) 在 50hz 时就是 PWM_DUTY_MAX/(1000/50)*(1+Angle/180)

// 那么 100hz 下 90度的打角 即高电平时间1.5ms 计算套用为
// PWM_DUTY_MAX/(1000/100)*(1+90/180) = PWM_DUTY_MAX/10*1.5

//舵机占空比计算方式
#define SERVO_MOTOR_DUTY(x)         ((float)PWM_DUTY_MAX/(1000.0/(float)SERVO_MOTOR_FREQ)*(0.5+(float)(x)/90.0))
#if (SERVO_MOTOR_FREQ<50 || SERVO_MOTOR_FREQ>300)
    #error "SERVO_MOTOR_FREQ ERROE!"
#endif

void Steer_Init(void);
void Steer_Set(uint32 Steer_duty);

#endif /* CODE_SERVO_H_ */



