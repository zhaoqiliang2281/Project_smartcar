/*
 * Motor.c
 *
 *  Created on: 2026年1月8日
 *      Author: LiangZhaoqi
 */

#include "Motor.h"

pid_type_def PID_Right_Speed_Encoder;
pid_type_def PID_Left_Speed_Encoder;
/************************************************
 *  函数名称：电机初始化
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年1月10日
 *  备    注：
 ************************************************/
void All_MOtor_init(void)
{

#if Motor_USE_Zhufei

    pwm_init(Timer_PWMA, 1000, 0);                       // PWM 通道初始化频率 10KHz 占空比初始为 0
    gpio_init(Back_Motor_DIR_PWMA, GPO, 0, GPO_PUSH_PULL);

    pwm_init(Timer_PWMB, 1000, 0);                       // PWM 通道初始化频率 10KHz 占空比初始为 0
    gpio_init(Back_Motor_DIR_PWMB, GPO, 0, GPO_PUSH_PULL);

#else

    pwm_init(Timer_PWMA, 10000, 0);                                                 // PWM 通道初始化频率 10KHz 占空比初始为 0
    pwm_init(Timer_PWMB, 10000, 0);                                                 // PWM 通道初始化频率 10KHz 占空比初始为 0
    pwm_init(Timer_PWMC, 10000, 0);                                                 // PWM 通道初始化频率 10KHz 占空比初始为 0
    pwm_init(Timer_PWMD, 10000, 0);                                                 // PWM 通道初始化频率 10KHz 占空比初始为 0

#endif

#if Dir_Motor_USE_Zhufei

    pwm_init(Timer_DIR_PWM, 17000, 0);                    // PWM 通道初始化频率 17KHz 占空比初始为 0
    gpio_init(Motor_DIR, GPO, 0, GPO_PUSH_PULL);          //初始化电机方向控制引脚

#else

    pwm_init(Timer_DIR_PWMA, 10000, 0);                                                 // PWM 通道初始化频率 10KHz 占空比初始为 0
    pwm_init(Timer_DIR_PWMB, 10000, 0);                                                 // PWM 通道初始化频率 10KHz 占空比初始为 0

#endif

    float Right_SpeedPID_PID_Value[3]={Right_Speed_Pid_KP,Right_Speed_Pid_KI,Right_Speed_Pid_KD};
    PID_init(&PID_Right_Speed_Encoder,PID_DELTA,Right_SpeedPID_PID_Value,800,600);
    float Left_SpeedPID_PID_Value[3]={Left_Speed_Pid_KP,Left_Speed_Pid_KI,Left_Speed_Pid_KD};
    PID_init(&PID_Left_Speed_Encoder,PID_DELTA,Left_SpeedPID_PID_Value,800,600);
}

/************************************************
 *  函数名称：电机限制PWM赋值
 *  功能说明：
 *  参数说明：
 *  参   数：电机PWM值,限幅最高PWM值
 *  函数返回：限制后的值
 *  修改时间：2026年1月11日
 *  备    注：内部调用，要想修改限幅值在Motor.h宏定义
 ************************************************/
static int32 MOtor_Limit_Pwm_Max(int32 MotoPwm,int32 Limit_Out)
{
    if(MotoPwm> Limit_Out) {MotoPwm = Limit_Out;}
    if(MotoPwm<-Limit_Out) {MotoPwm =-Limit_Out;}
    return MotoPwm;
}
/************************************************
 *  函数名称：将输入值在±threshold范围内清零
 *  功能说明：
 *  参数说明：
 *  参   数：电机PWM值,限幅最低PWM值
 *  函数返回：限制后的值
 *  修改时间：2026年4月21日
 *  备    注：内部调用
 ************************************************/
static void Motor_Limit_Pwm_Man(int *input, int threshold)
{
    if (abs(*input) < threshold)
    {
        *input = 0;
    }
}
/************************************************
 *  函数名称：方向电机PWM赋值
 *  功能说明：赋值给PWM底层电机
 *  参数说明：
 *  参   数：方向电机PWM值 电机PWM范围：+-Dir_MOtor_Limit_Max_Out_Pwm
 *  函数返回：无
 *  修改时间：2026年1月15日
 *  备    注：
 ************************************************/
void Dir_Motor_SetPwm(int32 PWM)
{

#if Dir_Motor_USE_Zhufei

//    PWM = MOtor_Limit_Pwm_Max(PWM,Dir_MOtor_Limit_Max_Out_Pwm);         /* 限幅 */
//    if(PWM>0)   /* 正转 */
//    {
//        gpio_set_level(Motor_DIR, GPIO_HIGH);
//        pwm_set_duty(Timer_DIR_PWM,PWM);
//    }
//    else if(PWM<0)  /* 反转 */
//    {
//        gpio_set_level(Motor_DIR, GPIO_LOW);
//        pwm_set_duty(Timer_DIR_PWM,-PWM);
//    }
//    else    /* 无输出，直接关闭驱动 */
//    {
//        pwm_set_duty(Timer_DIR_PWM,0);
//    }
    PWM = MOtor_Limit_Pwm_Max(PWM,Dir_MOtor_Limit_Max_Out_Pwm);         /* 限幅 */
    if(PWM>0)   /* 正转 */
    {
        gpio_set_level(Motor_DIR, GPIO_LOW);
        pwm_set_duty(Timer_DIR_PWM,PWM);
    }
    else if(PWM<0)  /* 反转 */
    {
        gpio_set_level(Motor_DIR, GPIO_HIGH);
        pwm_set_duty(Timer_DIR_PWM,-PWM);
    }
    else    /* 无输出，直接关闭驱动 */
    {
        pwm_set_duty(Timer_DIR_PWM,0);
    }

#else

    PWM=MOtor_Limit_Pwm_Max(PWM,Dir_MOtor_Limit_Max_Out_Pwm);  /* 限幅 */
    if(PWM>0)         /* 正转 */
    {
        pwm_set_duty(Timer_DIR_PWMA,PWM);
        pwm_set_duty(Timer_DIR_PWMB,0);
    }
    else if(PWM<0)    /* 反转 */
    {
        pwm_set_duty(Timer_DIR_PWMB,-PWM);
        pwm_set_duty(Timer_DIR_PWMA,0);
    }
    else                /* 无输出，直接关闭驱动 */
    {
        pwm_set_duty(Timer_DIR_PWMA,0);
        pwm_set_duty(Timer_DIR_PWMB,0);
    }

#endif

}

/************************************************
 *  函数名称：后驱右电机
 *  功能说明：
 *  参数说明：
 *  参   数：电机PWM值 电机PWM范围：-1000~1000
 *  函数返回：无
 *  修改时间：2026年1月15日
 *  备    注：内部调用
 ************************************************/
static void Back_MOtor_right(int32 PWM)
{

#if Motor_USE_Zhufei

    PWM=MOtor_Limit_Pwm_Max(PWM,Back_MOtor_Limit_Max_Out_Pwm);  /* 限幅 */
    if(PWM>0)         /* 正转 */
    {
        pwm_set_duty(Timer_PWMB,PWM);
        gpio_set_level(Back_Motor_DIR_PWMB, GPIO_LOW);
    }
    else if(PWM<0)    /* 反转 */
    {
        pwm_set_duty(Timer_PWMB,-PWM);
        gpio_set_level(Back_Motor_DIR_PWMB, GPIO_HIGH);
    }
    else                /* 无输出，直接关闭驱动 */
    {
        pwm_set_duty(Timer_PWMB,0);
    }

#else

    PWM=MOtor_Limit_Pwm_Max(PWM,Back_MOtor_Limit_Max_Out_Pwm);  /* 限幅 */
    if(PWM>0)         /* 正转 */
    {
        pwm_set_duty(Timer_PWMC,PWM);
        pwm_set_duty(Timer_PWMD,0);
    }
    else if(PWM<0)    /* 反转 */
    {
        pwm_set_duty(Timer_PWMD,-PWM);
        pwm_set_duty(Timer_PWMC,0);
    }
    else                /* 无输出，直接关闭驱动 */
    {
        pwm_set_duty(Timer_PWMC,0);
        pwm_set_duty(Timer_PWMD,0);
    }

#endif

}

/************************************************
 *  函数名称：后驱左电机
 *  功能说明：
 *  参数说明：
 *  参   数：电机PWM 范围：-1000~1000
 *  函数返回：无
 *  修改时间：2026年1月15日
 *  备    注：内部调用
 ************************************************/
static void Back_MOtor_left(int32 PWM)
{

#if Motor_USE_Zhufei

    PWM=MOtor_Limit_Pwm_Max(PWM,Back_MOtor_Limit_Max_Out_Pwm);  /* 限幅 */
    if(PWM>0)         /* 正转 */
    {
        pwm_set_duty(Timer_PWMA,PWM);
        gpio_set_level(Back_Motor_DIR_PWMA, GPIO_LOW);
    }
    else if(PWM<0)    /* 反转 */
    {
        pwm_set_duty(Timer_PWMA,-PWM);
        gpio_set_level(Back_Motor_DIR_PWMA, GPIO_HIGH);
    }
    else                /* 无输出，直接关闭驱动 */
    {
        pwm_set_duty(Timer_PWMA,0);
    }

#else

    PWM=MOtor_Limit_Pwm_Max(PWM,Back_MOtor_Limit_Max_Out_Pwm);  /* 限幅 */
    if(PWM>0)         /* 正转 */
    {
        pwm_set_duty(Timer_PWMA,PWM);
        pwm_set_duty(Timer_PWMB,0);
    }
    else if(PWM<0)    /* 反转 */
    {
        pwm_set_duty(Timer_PWMB,-PWM);
        pwm_set_duty(Timer_PWMA,0);
    }
    else                /* 无输出，直接关闭驱动 */
    {
        pwm_set_duty(Timer_PWMA,0);
        pwm_set_duty(Timer_PWMB,0);
    }

#endif

}

/************************************************
 *  函数名称：后电机PWM赋值
 *  功能说明：赋值给PWM底层电机
 *  参数说明：
 *  参   数：左右电机PWM值
 *  函数返回：无
 *  修改时间：2026年1月15日
 *  备    注：这里是底层电机驱动，不包含PID速度环，要速度环在PID.c中调用：Motor_Speed_Encoder_PID_Start
 ************************************************/
void Back_Motor_SetPwm(int32 Left_PWM,int32 Right_PWM)
{
    Motor_Limit_Pwm_Man(&Left_PWM ,20);
    Motor_Limit_Pwm_Man(&Right_PWM,20);
    Back_MOtor_left(Left_PWM);
    Back_MOtor_right(Right_PWM);
//    printf("%d,%d,%d\n\r",(int)Left_PWM,(int)Right_PWM,(int)0);//上位机看值
}

