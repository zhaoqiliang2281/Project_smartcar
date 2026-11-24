/*
 * Motor.c
 *
 *  Created on: 2026年1月8日
 *      Author: liang2281
 */

#include "zf_common_headfile.h"
#include "Motor.h"

/************************************************
 *  函数名称：电机初始化：void MOtor_init(void)
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年1月10日
 *  备    注：
 ************************************************/
void All_MOtor_init(void)
{

#if Mtor_USE_Zhufei

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

#if Dir_Mtor_USE_Zhufei

    pwm_init(Timer_DIR_PWM, 17000, 0);                    // PWM 通道初始化频率 17KHz 占空比初始为 0
    gpio_init(Motor_DIR, GPO, 0, GPO_PUSH_PULL);          //初始化电机方向控制引脚

#else

    pwm_init(Timer_DIR_PWMA, 10000, 0);                                                 // PWM 通道初始化频率 10KHz 占空比初始为 0
    pwm_init(Timer_DIR_PWMB, 10000, 0);                                                 // PWM 通道初始化频率 10KHz 占空比初始为 0

#endif

}

/************************************************
 *  函数名称：电机限制PWM赋值：int32 MOtor_Limit_Pwm(int32 MotoPwm,int32 Limit_Out)
 *  功能说明：
 *  参数说明：
 *  参   数：电机PWM值,限幅最高PWM值
 *  函数返回：限制后的值
 *  修改时间：2026年1月11日
 *  备    注：内部调用，要想修改限幅值在Motor.h宏定义
 ************************************************/
int32 MOtor_Limit_Pwm(int32 MotoPwm,int32 Limit_Out)
{
    if(MotoPwm> Limit_Out) {MotoPwm = Limit_Out;}
    if(MotoPwm<-Limit_Out) {MotoPwm =-Limit_Out;}
    return MotoPwm;
}

/************************************************
 *  函数名称：方向电机PWM赋值：void Dir_Motor_SetPwm(int32 PWM)
 *  功能说明：赋值给PWM底层电机
 *  参数说明：
 *  参   数：方向电机PWM值 电机PWM范围：+-Dir_MOtor_Limit_Max_Out_Pwm
 *  函数返回：无
 *  修改时间：2026年1月15日
 *  备    注：
 ************************************************/
void Dir_Motor_SetPwm(int32 PWM)
{

#if Dir_Mtor_USE_Zhufei

    PWM = MOtor_Limit_Pwm(PWM,Dir_MOtor_Limit_Max_Out_Pwm);         /* 限幅 */
    if(PWM>0)   /* 正转 */
    {
        gpio_set_level(Motor_DIR, GPIO_HIGH);
        pwm_set_duty(Timer_DIR_PWM,PWM);
    }
    else if(PWM<0)  /* 反转 */
    {
        gpio_set_level(Motor_DIR, GPIO_LOW);
        pwm_set_duty(Timer_DIR_PWM,-PWM);
    }
    else    /* 无输出，直接关闭驱动 */
    {
        pwm_set_duty(Timer_DIR_PWM,0);
    }


#else

    PWM=MOtor_Limit_Pwm(PWM,Dir_MOtor_Limit_Max_Out_Pwm);  /* 限幅 */
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
 *  函数名称：后驱右电机：Back_MOtor_right(int32 count)
 *  功能说明：
 *  参数说明：
 *  参   数：电机PWM值 电机PWM范围：-10000~10000
 *  函数返回：无
 *  修改时间：2026年1月15日
 *  备    注：
 ************************************************/
void Back_MOtor_right(int32 PWM)
{
#if Mtor_USE_Zhufei

    PWM=MOtor_Limit_Pwm(PWM,Back_MOtor_Limit_Max_Out_Pwm);  /* 限幅 */
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

    PWM=MOtor_Limit_Pwm(PWM,Back_MOtor_Limit_Max_Out_Pwm);  /* 限幅 */
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
 *  函数名称：后驱左电机：Back_MOtor_left(int32 count)
 *  功能说明：
 *  参数说明：
 *  参   数：电机PWM 范围：-10000~10000
 *  函数返回：无
 *  修改时间：2026年1月15日
 *  备    注：
 ************************************************/
void Back_MOtor_left(int32 PWM)
{

#if Mtor_USE_Zhufei

    PWM=MOtor_Limit_Pwm(PWM,Back_MOtor_Limit_Max_Out_Pwm);  /* 限幅 */
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

    PWM=MOtor_Limit_Pwm(PWM,Back_MOtor_Limit_Max_Out_Pwm);  /* 限幅 */
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
 *  函数名称：后电机PWM赋值：void Back_Motor_SetPwm(int32 left_PWM,int32 right_PWM)
 *  功能说明：赋值给PWM底层电机
 *  参数说明：
 *  参   数：左右电机PWM值
 *  函数返回：无
 *  修改时间：2026年1月15日
 *  备    注：
 ************************************************/
void Back_Motor_SetPwm(int32 Left_PWM,int32 Right_PWM)
{
    Back_MOtor_left(Left_PWM);
    Back_MOtor_right(Right_PWM);
//    printf("%d,%d,%d\n\r",(int)Left_PWM,(int)Right_PWM,(int)0);//上位机看值
}

