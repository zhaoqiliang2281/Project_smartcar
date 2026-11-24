/*
 * HotRC.c
 *
 *  Created on: 2025年6月15日
 *      Author: LiangZhaoqi
 */

#include "zf_common_headfile.h"

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
 *  函数名称：遥控器控制车运动
 *  功能说明：遥控接收到数据控制车运动
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月16日
 *  备    注：简单的控制车运动，没加线性
 ************************************************/
int32 HotRC_control_Speed_duty=0;    //遥控控制电机输出占空比
//遥控与刹车优先级第一
void HotRC_control(void)
{
    if(uart_receiver.channel[3] <= 192)//速度输出设置
    {
        HotRC_control_Speed_duty = 200; //设置档位1
    }
    else if(uart_receiver.channel[3] == 992)
    {
        HotRC_control_Speed_duty = 400; //设置档位2
    }
    else if(uart_receiver.channel[3] == 1792) //遥控换到最大是手动控制
    {
        HotRC_control_Speed_duty = 600; //设置档位3
    }
    else
    {

        HotRC_control_Speed_duty = 0;
    }
    //遥控接收到数据控制车运动
    if(uart_receiver.channel[1]>1500)
    {
        Back_Motor_SetPwm(HotRC_control_Speed_duty,HotRC_control_Speed_duty);//判断是标志位就给速度
    }
    else if(uart_receiver.channel[1]<500)
    {
        Back_Motor_SetPwm(-HotRC_control_Speed_duty,-HotRC_control_Speed_duty);
    }
    else
    {
        Back_Motor_SetPwm(0,0);
    }

    //左右控制
    if(uart_receiver.channel[0]>1500)
    {
        Angle_Target_Position = 800;//右
    }
    else if(uart_receiver.channel[0]<150)
    {
        Angle_Target_Position = -800;//左
    }
    else if(uart_receiver.channel[0]==848)
    {
        Angle_Target_Position = 0;
    }
}
