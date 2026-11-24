/*
 * HotRC.c
 *
 *  Created on: 2025年6月15日
 *      Author: 吉祥止止
 */

#include "zf_common_headfile.h"

/************************************************
 *  函数名称：遥控器初始化：HotRC_control_Init(void)
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
 *  函数名称：HotRC遥控接收数据显示：HotRC_receive_data_Show(void)
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月15日
 *  备    注：
 ************************************************/
int8 HotRC_control_Speed_duty=0;    //遥控控制电机输出占空比
int8 HotRC_control_Speed_duty1=0;    //电机输出占空比设置大小
int8 HotRC_control_servo_duty=0;    //遥控控制舵机输出占空比

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
 *  函数名称：遥控器控制车运动：HotRC_control(void)
 *  功能说明：遥控接收到数据控制车运动
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月16日
 *  备    注：
 ************************************************/

void HotRC_control(void)
{
    //速度输出设置
    if(uart_receiver.channel[3]<=195)
    {
        HotRC_control_Speed_duty1 = 10;
    }
    else if(uart_receiver.channel[3]==992)
    {
        HotRC_control_Speed_duty1 = 15;
    }
    else if(uart_receiver.channel[3]==1792)
    {
        HotRC_control_Speed_duty1 = 20;
    }
    else
    {
        HotRC_control_Speed_duty1 = 0;
    }
    //遥控接收到数据控制车运动(油门通道 channel[1] 线性输出)
    //油门通道范围: 中心 1136, 前进最大 1936, 后退最大 497
    int32 channel1 = (int32)uart_receiver.channel[1];

    //限幅, 防止通道值超范围
    if(channel1 > 1936)
    {
        channel1 = 1936;
    }
    if(channel1 < 497)
    {
        channel1 = 497;
    }

    if(channel1 > 1136)
    {
        //前进: 1136~1936 线性映射到 0~Speed_duty1
        HotRC_control_Speed_duty = (int8)((channel1 - 1136) * HotRC_control_Speed_duty1 / 800);
    }
    else if(channel1 < 1136)
    {
        //后退: 497~1136 线性映射到 -Speed_duty1~0
        HotRC_control_Speed_duty = (int8)((channel1 - 1136) * HotRC_control_Speed_duty1 / 639);
    }
    else
    {
        HotRC_control_Speed_duty = 0;
    }

    //左右舵机控制(转向通道 channel[0] 线性输出)
    //转向通道范围: 中心 824, 右最大 1624, 左最大 24
    int32 channel0 = (int32)uart_receiver.channel[0];

    //限幅, 防止通道值超范围
    if(channel0 > 1624)
    {
        channel0 = 1624;
    }
    if(channel0 < 24)
    {
        channel0 = 24;
    }

    if(channel0 > 824)
    {
        //右转: 824~1624 线性映射到 SERVO_MOTOR_MID~SERVO_MOTOR_R_MAX
        HotRC_control_servo_duty = (int8)(SERVO_MOTOR_MID + (channel0 - 824) * (SERVO_MOTOR_R_MAX - SERVO_MOTOR_MID) / 800);
    }
    else if(channel0 < 824)
    {
        //左转: 24~824 线性映射到 SERVO_MOTOR_L_MAX~SERVO_MOTOR_MID
        HotRC_control_servo_duty = (int8)(SERVO_MOTOR_MID - (824 - channel0) * (SERVO_MOTOR_MID - SERVO_MOTOR_L_MAX) / 800);
    }
    else
    {
        HotRC_control_servo_duty = SERVO_MOTOR_MID;
    }
    //显示电机与舵机占空比
    ips200_show_int(  80 , 16*10,  HotRC_control_Speed_duty,  5);
    ips200_show_int(  80 , 16*11,  HotRC_control_servo_duty,  5);

    //电机编码速度
    ips200_show_int( 80 , 16*12,  encoder[0], 8);

    //电机占空比赋值
    Pwm_Motor(HotRC_control_Speed_duty);

    //舵机占空比赋值
    Steer_Set(HotRC_control_servo_duty);
}



