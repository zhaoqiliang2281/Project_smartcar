/*
 * HotRC.c
 *
 *  Created on: 2025年6月15日
 *      Author: LiangZhaoqi
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
        HotRC_control_Speed_duty1 = 5;
    }
    //遥控接收到数据控制车运动
    if(uart_receiver.channel[1]>1200)
    {
        HotRC_control_Speed_duty = HotRC_control_Speed_duty1;//判断是标志位就给速度
    }
    else if(uart_receiver.channel[1]<1000)
    {
        HotRC_control_Speed_duty = -HotRC_control_Speed_duty1;
    }
    else
    {
        HotRC_control_Speed_duty=5;
    }

    //左右舵机控制
    if(uart_receiver.channel[0]>1000)
    {
        HotRC_control_servo_duty = SERVO_MOTOR_R_MAX;
    }
    else if(uart_receiver.channel[0]<700)
    {
        HotRC_control_servo_duty = SERVO_MOTOR_L_MAX;
    }
    else
    {
        HotRC_control_servo_duty=95;
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



