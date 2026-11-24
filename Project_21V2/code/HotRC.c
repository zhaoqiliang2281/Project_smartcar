/*
 * HotRC.c
 *
 *  Created on: 2025年6月15日
 *      Author: LiangZhaoqi
 */

#include "zf_common_headfile.h"

static int32 HotRC_control_Speed_duty = 0; //遥控控制速度档位

/************************************************
 *  函数名称：遥控器初始化
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月15日
 *  备    注：
 ************************************************/
void HotRC_control_Init(void)
{
    uart_receiver_init();       //sbus接收机初始化
}

/************************************************
 *  函数名称：HotRC遥控接收数据显示
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月15日
 *  备    注：
 ************************************************/
void HotRC_receive_data_Show(void)
{
    //HotRC遥控接收数据
    if(1 == uart_receiver.finsh_flag)                            // 帧完成标志判断
    {
        if(1 == uart_receiver.state)                             // 遥控器失控状态判断
        {
            ips200_show_int(200,16*0,uart_receiver.channel[0],4);// 输出6个通道数据
            ips200_show_int(200,16*1,uart_receiver.channel[1],4);
            ips200_show_int(200,16*2,uart_receiver.channel[2],4);
            ips200_show_int(200,16*3,uart_receiver.channel[3],4);
            ips200_show_int(200,16*4,uart_receiver.channel[4],4);
            ips200_show_int(200,16*5,uart_receiver.channel[5],4);
            ips200_show_string(190, 16*6, "OKmote");
        }
        else
        {
            ips200_show_string(190, 16*6, "Remote"); // 串口输出失控提示
        }
        uart_receiver.finsh_flag = 0;                // 帧完成标志复位
    }
}

/************************************************
 *  函数名称: 通道3负向线性映射
 *  功能说明：将 uart_receiver.channel[3] 值线性映射到 -1000 - 0
 *  参数说明：ch_val 原始通道值 (CH3_RAW_MIN - CH3_RAW_MID)
 *  参   数：原始通道值
 *  函数返回：将通道值线性映射值（负方向）
 *  修改时间：2026年8月10日
 *  备   注：按偏离中位 CH3_RAW_MID 的幅度映射，越往后拉速度越大
 *           CH3_RAW_MIN 对应 -1000，CH3_RAW_MID(中位) 对应 0
 ************************************************/
static int16_t map_channel3_negative(uint16_t ch_val)
{
    const int16_t CENTER    = CH3_RAW_MID;      // 摇杆中位
    const int16_t IN_MIN    = CH3_RAW_MIN;      // 最大后退
    const int16_t OUT_MAX   = 500;
    const int16_t HALF_RANGE = (CENTER - IN_MIN); // 800

    if (ch_val >= CENTER) return 0;        // 中位及以上：不后退
    if (ch_val <= IN_MIN) return -OUT_MAX; // 拉满：-1000

    int32_t numerator = ((int32_t)CENTER - ch_val) * OUT_MAX; // 偏离中位的量
    int32_t result = (numerator + (HALF_RANGE / 2)) / HALF_RANGE;

    return -(int16_t)result;
}

/************************************************
 *  函数名称: 通道3正向线性映射
 *  功能说明：将 uart_receiver.channel[3] 值线性映射到 0 - 1000
 *  参数说明：ch_val 原始通道值 (CH3_RAW_MID - CH3_RAW_MAX)
 *  参   数：原始通道值
 *  函数返回：将通道值线性映射值（正方向）
 *  修改时间：2026年8月10日
 *  备   注：CH3_RAW_MID - CH3_RAW_MAX 对应 0 - 1000
 ************************************************/
static uint16_t map_channel3_positive(uint16_t ch_val)
{
    const int16_t IN_MIN  = CH3_RAW_MID; // 中位
    const int16_t IN_MAX  = CH3_RAW_MAX; // 最大前进
    const int16_t OUT_MIN = 0;
    const int16_t OUT_MAX = 500;

    if (ch_val <= IN_MIN) return OUT_MIN;
    if (ch_val >= IN_MAX) return OUT_MAX;

    int32_t numerator = ((int32_t)ch_val - IN_MIN) * (OUT_MAX - OUT_MIN);
    int32_t denominator = (IN_MAX - IN_MIN); // 800

    int32_t result = (numerator + (denominator / 2)) / denominator;

    return (uint16_t)result;
}

/************************************************
 *  函数名称: 通道0负向线性映射（左转）
 *  功能说明：将 uart_receiver.channel[0] 值线性映射到 -MAXIMUM_LEFT_AND_RIGHT_STEERING_ANGLE - 0
 *  参数说明：ch_val 原始通道值 (CH0_RAW_MIN - CH0_RAW_MID)
 *  参   数：原始通道值
 *  函数返回：将通道值线性映射值（负方向）
 *  修改时间：2026年8月10日
 *  备   注：按偏离中位 CH0_RAW_MID 的幅度映射，越往左打转向越大
 *           CH0_RAW_MIN 对应 -MAXIMUM_LEFT_AND_RIGHT_STEERING_ANGLE，
 *           CH0_RAW_MID(中位) 对应 0
 ************************************************/
static int16_t map_channel0_negative(uint16_t ch_val)
{
    const int16_t CENTER    = CH0_RAW_MID;      // 摇杆中位
    const int16_t IN_MIN    = CH0_RAW_MIN;      // 最大左转
    const int16_t OUT_MAX = MAXIMUM_LEFT_AND_RIGHT_STEERING_ANGLE;
    const int16_t HALF_RANGE = (CENTER - IN_MIN); // 800

    if (ch_val >= CENTER) return 0;        // 中位及以上：不左转
    if (ch_val <= IN_MIN) return -OUT_MAX; // 打满：-MAXIMUM_LEFT_AND_RIGHT_STEERING_ANGLE

    int32_t numerator = ((int32_t)CENTER - ch_val) * OUT_MAX; // 偏离中位的量
    int32_t result = (numerator + (HALF_RANGE / 2)) / HALF_RANGE;

    return -(int16_t)result;
}

/************************************************
 *  函数名称: 通道0正向线性映射（右转）
 *  功能说明：将 uart_receiver.channel[0] 值线性映射到 0 - MAXIMUM_LEFT_AND_RIGHT_STEERING_ANGLE
 *  参数说明：ch_val 原始通道值 (CH0_RAW_MID - CH0_RAW_MAX)
 *  参   数：原始通道值
 *  函数返回：将通道值线性映射值（正方向）
 *  修改时间：2026年8月10日
 *  备   注：CH0_RAW_MID - CH0_RAW_MAX 对应 0 - MAXIMUM_LEFT_AND_RIGHT_STEERING_ANGLE
 ************************************************/
static uint16_t map_channel0_positive(uint16_t ch_val)
{
    const int16_t IN_MIN  = CH0_RAW_MID; // 中位
    const int16_t IN_MAX  = CH0_RAW_MAX; // 最大右转
    const int16_t OUT_MIN = 0;
    const int16_t OUT_MAX = MAXIMUM_LEFT_AND_RIGHT_STEERING_ANGLE;

    if (ch_val <= IN_MIN) return OUT_MIN;
    if (ch_val >= IN_MAX) return OUT_MAX;

    int32_t numerator = ((int32_t)ch_val - IN_MIN) * (OUT_MAX - OUT_MIN);
    int32_t denominator = (IN_MAX - IN_MIN); // 800

    int32_t result = (numerator + (denominator / 2)) / denominator;

    return (uint16_t)result;
}

/************************************************
 *  函数名称：遥控器控制车运动
 *  功能说明：遥控接收到数据控制车运动
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月16日
 *  备    注：
 ************************************************/
void HotRC_control(void)
{
    if (uart_receiver.state == 1)
    {
        // 遥控接收到数据控制车运动
        if (uart_receiver.channel[1] < CH3_RAW_MID) // 低于中位：负方向
        {
            HotRC_control_Speed_duty = map_channel3_negative(uart_receiver.channel[1]); // -1000 ~ 0
        }
        else // 高于或等于中位：正方向
        {
            HotRC_control_Speed_duty = map_channel3_positive(uart_receiver.channel[1]); // 0 ~ 1000
        }
        Back_Motor_SetPwm(HotRC_control_Speed_duty, HotRC_control_Speed_duty); // 设置电机PWM

        // 左右控制（线性映射）
        if (uart_receiver.channel[0] < CH0_RAW_MID) // 低于中位：左转
        {
            Angle_Target_Position = map_channel0_negative(uart_receiver.channel[0]); // -MAXIMUM_LEFT_AND_RIGHT_STEERING_ANGLE ~ 0
        }
        else // 高于或等于中位：右转
        {
            Angle_Target_Position = map_channel0_positive(uart_receiver.channel[0]); // 0 ~ MAXIMUM_LEFT_AND_RIGHT_STEERING_ANGLE
        }
    }
    else if(uart_receiver.state == 0)
    {
        uart_receiver.channel[0] = 0;
        uart_receiver.channel[1] = 0;
        uart_receiver.channel[2] = 0;
        uart_receiver.channel[3] = 0;
        uart_receiver.channel[4] = 0;
        uart_receiver.channel[5] = 0;
    }
}
