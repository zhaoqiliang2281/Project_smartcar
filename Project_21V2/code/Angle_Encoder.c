/*
 * Angle_Encoder.c
 *
 *  Created on: 2026年4月22日
 *      Author: LiangZhaoqi
 */

#include "Angle_Encoder.h"

MeanFilter_TypeDef Angle_Encoder_MeanFilter;

int32 Angle_Target_Position = 0; // 目标位置 //目标位置 范围：+-937

static int32 Angle_Motor = 0;       // PID计算出的电机PWM值
static int16 Angle_Encoder_cnt = 0; // 累计编码值

#if Dir_Mtor_USE_PID_STRUCT

#if Dir_Mtor_USE_PID_ENCODER

// 12V
// float Position_KP = 58.95f;
// float Position_KI = 5.45f;
// float Position_KD = 0.01f;

// 24V
static float Position_KP = 28.95f;
static float Position_KI = 0.0f;
static float Position_KD = 0.25f;

/************************************************
 *  函数名称：方向电机转向位置式PID控制器
 *  功能说明：
 *  参数说明：
 *  参   数：实际位置，目标位置
 *  函数返回：电机PWM
 *  修改时间：2026年1月16日
 *  备    注：根据位置式离散PID公式 Kp*e(k)+Ki*∑e(k)+Kd[e（k）-e(k-1)]
 ************************************************/
static int32_t Dir_Motor_Position_PID_Control(int16 reality, int16 target)
{
    static float Bias = 0, Pwm = 0, Last_Bias = 0, Integral_bias = 0;

    Bias = target - reality;
    Integral_bias += Bias;

    if (Integral_bias > 2000)
        Integral_bias = 2000;
    if (Integral_bias < -2000)
        Integral_bias = -2000;

    Pwm = (Position_KP * Bias) + (Position_KI * Integral_bias) + (Position_KD * (Bias - Last_Bias));

    Last_Bias = Bias;
    return Pwm;
}

static int32 Target_Position = 0; // 目标位置

/************************************************
 *  函数名称：定时器计算编码器累积值与转向电机PID
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年1月15日
 *  备    注：中断10ms调用
 ************************************************/
void Dir_Motor_Encoder_PID_Update(void)
{
    Encoder_cnt += Read_Encoder(TIM4_ENCODER);

    current_angle = 4 * Encoder_cnt;

    int16 error = current_angle - Target_Position;

    if (abs(error) < 1 && abs(Motor) < 10)
    {
        Dir_Motor_SetPwm(0);
        Motor = 0;
    }
    else
    {
        Motor = (int32)Dir_Motor_Position_PID_Control(current_angle, (int16)Target_Position);
        Dir_Motor_SetPwm(Motor);
    }
    //    printf("%d,%d,%d\n\r",(int)Motor,current_angle,(int)Target_Position);/*上位机看值,调参代码，勿删*/
}
#else
#endif

static float Angle_Position_KP = 6.68f;
static float Angle_Position_KI = 0.1f;
static float Angle_Position_KD = 0.01f;

/************************************************
 *  函数名称：前转向角度编码器初始化
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年4月23日
 *  备    注：
 ************************************************/
void Angle_Encoder_Init(void)
{
    absolute_encoder_init();
    MeanFilter_Init(&Angle_Encoder_MeanFilter);
}

/************************************************
 *  函数名称：前转向角度编码器PID控制器
 *  功能说明：
 *  参数说明：
 *  参   数：reality：实际值
 *          target：目标值
 *  函数返回：PID计算结果
 *  修改时间：2026年4月24日
 *  备    注：
 ************************************************/
int32_t Angle_Dir_Motor_Position_PID_Control(int16 reality, int16 target)
{
    static float Bias = 0, Result = 0, Last_Bias = 0, Integral_bias = 0;

    Bias = target - reality;
    Integral_bias += Bias;

    if (Integral_bias > 800)
        Integral_bias = 800;
    if (Integral_bias < -800)
        Integral_bias = -800;

    Integral_bias1 = Integral_bias;

    Result = (Angle_Position_KP * Bias) + (Angle_Position_KI * Integral_bias) + (Angle_Position_KD * (Bias - Last_Bias));

    Last_Bias = Bias;
    if (Result > 937)
        Result = 937;
    if (Result < -937)
        Result = -937;
    return Result;
}

/************************************************
 *  函数名称：前转向角度编码器PID输出
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年4月24日
 *  备    注：中断20ms调用
 ************************************************/
void Angle_Encoder_PID_Update(void)
{
    int16 raw_value = absolute_encoder_get_location();

    int16 mapped_value = raw_value - LEFT_ANGLE_ENCODER_LIMIT_MAX;

    int16 inverted_value = -mapped_value;

    Angle_Encoder_cnt = inverted_value + RIGHT_ANGLE_ENCODER_LIMIT_MAX;

    Angle_Target_Position = LEFT_ANGLE_ENCODER_MID_VALUE + Angle_Target_Position;
    int16 error = Angle_Encoder_cnt - Angle_Target_Position;

    if (abs(error) < 1 && abs(Angle_Motor) < 2)
    {
        Dir_Motor_SetPwm(0);
        Angle_Motor = 0;
    }
    else
    {
        Angle_Motor = (int32)Angle_Dir_Motor_Position_PID_Control(Angle_Encoder_cnt, (int16)Angle_Target_Position);
        Dir_Motor_SetPwm(Angle_Motor);
    }
}

#else

pid_type_def Angle_Dir_Encoder;
/************************************************
 *  函数名称：前转向角度编码器初始化
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年4月23日
 *  备    注：
 ************************************************/
void Angle_Encoder_Init(void)
{
    float Angle_Encoder_PID_Value[3] = {ANGLE_ENCODER_PID_KP, ANGLE_ENCODER_PID_KI, ANGLE_ENCODER_PID_KD};
    PID_init(&Angle_Dir_Encoder, PID_LOCATION, Angle_Encoder_PID_Value, MAXIMUM_LEFT_AND_RIGHT_STEERING_ANGLE, 800);

    absolute_encoder_init();

    MeanFilter_Init(&Angle_Encoder_MeanFilter);
}

/************************************************
 *  函数名称：前转向角度编码器PID控制
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年4月24日
 *  备    注：中断20ms调用
 ************************************************/
void Angle_Encoder_PID_Update(void)
{
    int16 raw_value = absolute_encoder_get_location();

    int16 mapped_value = raw_value - LEFT_ANGLE_ENCODER_LIMIT_MAX;

    int16 inverted_value = -mapped_value;

    Angle_Encoder_cnt = inverted_value + RIGHT_ANGLE_ENCODER_LIMIT_MAX;

    int16 Angle_Target_Position_Value = ANGLE_ENCODER_MID_VALUE + Angle_Target_Position;

    int16 error = Angle_Encoder_cnt - Angle_Target_Position_Value;

    if (abs(error) < 2 && abs(Angle_Motor) < 10)
    {
        Dir_Motor_SetPwm(0); // 关闭电机
        Angle_Motor = 0;
    }
    else
    {
        Angle_Motor = (int32)PID_Update(&Angle_Dir_Encoder, Angle_Encoder_cnt, (int16)Angle_Target_Position_Value); // PID计算
        Dir_Motor_SetPwm(Angle_Motor);
    }
}

#endif

/************************************************
 *  函数名称：获取前转向角度编码值
 *  功能说明：
 *          将 Angle_Encoder_cnt 从原始范围 [LEFT_ANGLE_ENCODER_LIMIT_MAX, RIGHT_ANGLE_ENCODER_LIMIT_MAX]
            线性映射到 [-MAXIMUM_LEFT_AND_RIGHT_STEERING_ANGLE, MAXIMUM_LEFT_AND_RIGHT_STEERING_ANGLE]
 *  参数说明：
 *  参   数：无
 *  函数返回：前转向角度编码值
 *  修改时间：2026年6月13日
 *  备    注：读出角度满足左边负、右边正,与PID计算输入角度值一致
 ************************************************/
int16 Angle_Encoder_Get_Value(void)
{
    // 编码器原始值减去中值即为偏移量，与 Angle_Target_Position 使用同一坐标系（1:1），无需缩放
    int16 centered = Angle_Encoder_cnt - ANGLE_ENCODER_MID_VALUE;
    if (centered > MAXIMUM_LEFT_AND_RIGHT_STEERING_ANGLE)
        centered = MAXIMUM_LEFT_AND_RIGHT_STEERING_ANGLE;
    if (centered < -MAXIMUM_LEFT_AND_RIGHT_STEERING_ANGLE)
        centered = -MAXIMUM_LEFT_AND_RIGHT_STEERING_ANGLE;
    return centered;
}

/************************************************
 *  函数名称：前转向角度编码器参数显示
 *  功能说明：用于测试
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年6月12日
 *  备    注：使用串口调试
 ************************************************/
void Angle_Encoder_Test_Show(void)
{
    /*串口解析：接收数据为整形，串口助手以文本格式发送了，m%dx%dy%dz%d ；其中‘%d’为对应函数参数,能同时解析四个整数*/
    Serial_Data_Resolving(&m, &x, &y, &z);
    ips200_show_int(0, 16 * 9, m, 5);
    ips200_show_int(0, 16 * 10, x, 5);
    ips200_show_int(0, 16 * 11, y, 5);
    ips200_show_int(0, 16 * 12, z, 5);

    // 前转向测试
     Angle_Target_Position = x;
//    int16 angle_value = Angle_Encoder_Get_Value(); // 获取前转向角度编码值
     int16 angle_value = Angle_Encoder_Get_Value(); // 获取前转向角度编码值
    printf("%d,%d,%d,%d\n\r", (int)m, x, (int)y, (int)z);

    // 实际值               //目标值                //电机输出值    //保留
    printf("%d,%d,%d,%d\n\r", (int)Angle_Encoder_cnt, Angle_Target_Position, (int)Angle_Motor, (int)angle_value); // 上位机看值
}
