/*
 * Motor.c
 *
 *  Created on: 2026年1月8日
 *      Author: LiangZhaoqi
 */

#include "Motor.h"
#include <math.h>

pid_type_def PID_Right_Speed_Encoder;
pid_type_def PID_Left_Speed_Encoder;

//VOFA数据中转
static int32 Left_Speed  = 0;   
static int32 Right_Speed = 0;

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
    PID_init(&PID_Right_Speed_Encoder, PID_DELTA, Right_SpeedPID_PID_Value, Back_MOtor_Limit_Max_Out_Pwm, 600);
    float Left_SpeedPID_PID_Value[3]={Left_Speed_Pid_KP,Left_Speed_Pid_KI,Left_Speed_Pid_KD};
    PID_init(&PID_Left_Speed_Encoder, PID_DELTA, Left_SpeedPID_PID_Value, Back_MOtor_Limit_Max_Out_Pwm, 600);
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
void Back_MOtor_right(int32 PWM)
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
void Back_MOtor_left(int32 PWM)
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
 *  函数名称: 线性映射 -1000~+1000 → -200~+200
 *  功能说明：将带符号输入值线性映射到带符号输出
 *  参数说明：val 原始值 (-1000 ~ +1000)
 *  函数返回：映射后的值 (-200 ~ +200)
 *  修改时间：2026年8月13日
 *  备   注：-1000 → -200，0 → 0，+1000 → +200
 ************************************************/
int32 map_1000_to_200(int32 val)
{
    const int32 IN_MAX  = 1000;
    const int32 OUT_MAX = 200;

    int32 abs_val = (val < 0) ? -val : val; // 取绝对值，保证四舍五入方向正确
    if (abs_val >= IN_MAX)
        abs_val = IN_MAX; // 上限钳位

    int32 result = (abs_val * OUT_MAX + (IN_MAX / 2)) / IN_MAX; // 四舍五入，abs_val*OUT_MAX 最大 180000 不溢出

    return (val < 0) ? -result : result; // 按原符号还原
}

/************************************************
 *  函数名称：后电机PWM赋值
 *  功能说明：赋值给PWM底层电机
 *  参数说明：
 *  参   数：左右电机PWM值
 *  函数返回：无
 *  修改时间：2026年1月15日
 *  备    注：这里是底层电机驱动，包含PID速度环，要速度环在PID.h修改宏Motor_Speed_Encoder_PID_Start
 ************************************************/
void Back_Motor_SetPwm(int32 Left_PWM, int32 Right_PWM)
{
    if (Motor_Start == 0)
    {
        Left_PWM = 0;
        Right_PWM = 0;
    }
#if Motor_USE_PID

    Motor_Limit_Pwm_Man(&Left_PWM, 20); // 目标死区：±1000 量程，20 = 2%
    Motor_Limit_Pwm_Man(&Right_PWM, 20);

    int32 Temp_Left_PWM =  map_1000_to_200(Left_PWM);
    int32 Temp_Right_PWM = map_1000_to_200(Right_PWM);

    Left_Speed = Temp_Left_PWM; // 传出到Vofa
    Right_Speed = Temp_Right_PWM;

    // 目标为 0：清积分和输出，避免起步冲击
    if (Temp_Left_PWM == 0)
    {
        PID_clear(&PID_Left_Speed_Encoder);
    }
    if (Temp_Right_PWM == 0)
    {
        PID_clear(&PID_Right_Speed_Encoder);
    }

    Motor_Speed_Encoder_PID_Start(Open, Temp_Left_PWM, Temp_Right_PWM);

    int32 PID_Left_Speed = (int32)PID_Left_Speed_Encoder.out;
    int32 PID_Right_Speed = (int32)PID_Right_Speed_Encoder.out;

    Back_MOtor_left(PID_Left_Speed);
    Back_MOtor_right(PID_Right_Speed);

#else

#if Motor_USE_DIFFERENTIAL
    /*加差速*/
    Motor_Limit_Pwm_Man(&Left_PWM, 20);
    Motor_Limit_Pwm_Man(&Right_PWM, 20);
    int32 Differential_Left;
    int32 Differential_Right;

    Differential_Motor(Left_PWM, Right_PWM, &Differential_Left, &Differential_Right);
    Back_MOtor_left(Differential_Left);
    Back_MOtor_right(Differential_Right);

    Left_Speed = Differential_Left; // 传出到Vofa
    Right_Speed = Differential_Right;
#else
    /*没有用差速的*/
     Motor_Limit_Pwm_Man(&Left_PWM, 20);
     Motor_Limit_Pwm_Man(&Right_PWM, 20);
     Back_MOtor_left(Left_PWM);
     Back_MOtor_right(Right_PWM);

     Left_Speed = Left_PWM; // 传出到Vofa
     Right_Speed = Right_PWM;

#endif

#endif
}

/************************************************
 *  函数名称：差速线性映射
 *  功能说明：根据前轮转向角对左右后轮基础速度做差速线性映射
 *  参数说明：
 *  参   数：Left_PWM/Right_PWM  - 输入基础左右电机速度
 *          *Differential_Left_PWM 除 *Differential_Right_PWM - 输出差速后的左右电机速度
 *  函数返回：无
 *  修改时间：2026年8月19日
 *  备    注：
 *          转向角范围：-937(最左) ~ 0(直行) ~ +937(最右)，正角=右转，负角=左转
 *          内轮减速、外轮加速，平均速度基本不变，全程线性映射
 *          满舵时差速比例由 Motor.h 中 DIFFERENTIAL_MAX_RATIO 决定(0~1)
 ************************************************/
void Differential_Motor(int32 Left_PWM, int32 Right_PWM, int32 *Differential_Left_PWM, int32 *Differential_Right_PWM)
{
    /* 前轮转向角：负=左转，0=直行，正=右转 */
    int16 Angle = Angle_Encoder_Get_Value();

    /* 转向角线性归一化到 [-1.0, +1.0] */
    float Ratio = (float)Angle / (float)MAXIMUM_LEFT_AND_RIGHT_STEERING_ANGLE;

    /* 差速比例：直行(0)时为0，满舵(±937)时为 DIFFERENTIAL_MAX_RATIO */
    float K = fabsf(Ratio) * DIFFERENTIAL_MAX_RATIO;

    if (Angle >= 0)   /* 右转：右侧为内轮(减速)，左侧为外轮(加速) */
    {
        *Differential_Left_PWM  = (int32)((float)Left_PWM  * (1.0f + K));
        *Differential_Right_PWM = (int32)((float)Right_PWM * (1.0f - K));
    }
    else              /* 左转：左侧为内轮(减速)，右侧为外轮(加速) */
    {
        *Differential_Left_PWM  = (int32)((float)Left_PWM  * (1.0f - K));
        *Differential_Right_PWM = (int32)((float)Right_PWM * (1.0f + K));
    }
}



/************************************************
 *  函数名称：VOFA回传后驱电机PID数据
 *  功能说明：将左右电机PID的关键数据填入缓冲区，供VOFA上位机显示
 *  参数说明：
 *  参   数：buf      - float数组指针
 *            buf_size - 数组实际可用元素个数
 *  函数返回：实际填入的数据个数
 *  修改时间：2026年8月13日
 *  备    注：内部自动限制写入范围，防止越界
 ************************************************/
/* VOFA通道布局：
 *  右电机: [0]Target [1]Actual [2]out [3]Pout [4]Iout [5]Dout
 *  左电机: [6]Target [7]Actual [8]out [9]Pout [10]Iout [11]Dout
 */
#define VOFA_MOTOR_PID_DATA_NUM     14u

uint8 Vofa_Back_Motor_PID_Data(float *buf, uint8 buf_size)
{
    if (buf == NULL || buf_size < VOFA_MOTOR_PID_DATA_NUM)
    {
        return 0u;
    }

    /* ---- 右电机PID ---- */
    buf[0]  = (float)PID_Right_Speed_Encoder.Target;   /* 右电机 目标速度 */
    buf[1]  = (float)PID_Right_Speed_Encoder.Actual;   /* 右电机 实际速度 */
    buf[2]  = PID_Right_Speed_Encoder.out;             /* 右电机 PID总输出 */
    buf[3]  = PID_Right_Speed_Encoder.Pout;            /* 右电机 P分量 */
    buf[4]  = PID_Right_Speed_Encoder.Iout;            /* 右电机 I分量 */
    buf[5]  = PID_Right_Speed_Encoder.Dout;            /* 右电机 D分量 */

    /* ---- 左电机PID ---- */
    buf[6]  = (float)PID_Left_Speed_Encoder.Target;    /* 左电机 目标速度 */
    buf[7]  = (float)PID_Left_Speed_Encoder.Actual;    /* 左电机 实际速度 */
    buf[8]  = PID_Left_Speed_Encoder.out;              /* 左电机 PID总输出 */
    buf[9]  = PID_Left_Speed_Encoder.Pout;             /* 左电机 P分量 */
    buf[10] = PID_Left_Speed_Encoder.Iout;             /* 左电机 I分量 */
    buf[11] = PID_Left_Speed_Encoder.Dout;             /* 左电机 D分量 */

    buf[12] = (float)Left_Speed;                       /* 左电机函数传入参数 */
    buf[13] = (float)Right_Speed;                      /* 左电机函数传入参数 */

    return VOFA_MOTOR_PID_DATA_NUM;
}
