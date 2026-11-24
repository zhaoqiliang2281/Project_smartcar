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
//uint8_t Key6_Num,Key7_Num,Key8_Num;
//
//uint8 Transform = 0;
//uint8 Motor_Start = 0;  //电机开启标志位
//
//uint8 Motor_Close_Temp = 0;

//uint8 sdyk;
//int16 HotRC_control_Speed_duty=0;    //遥控控制电机输出占空比
//遥控与刹车优先级第一
void HotRC_control(void)
{
//    Key6_Num = gpio_get_level(KEY6);    //脚踏  Key6_Num = 1 松开脚踏
//    Key7_Num = gpio_get_level(KEY7);    //自动(科目)与手动(遥控，脚踏)切换//Key7_Num=1为手动(遥控，脚踏)
//    Key8_Num = gpio_get_level(KEY8);    //手动情况下前后方向切换
//
//    /*遥控与脚踏独立控制车关停*/
//    if(Key6_Num == 0 || uart_receiver.channel[2] == 1792)//遥控与刹车优先级第一
//    {
//        ips200_show_string(120, 16*2, "Stop");
//        Motor_Start = 0;    //关闭电机
//        Motor_Close_Temp = 1;   //清除由低通滤波产生的滞后影响
//    }
//    else if(Key6_Num == 1 || uart_receiver.channel[2] == 192)
//    {
//        ips200_show_string(120, 16*2, "Go  ");
//        Motor_Start = 1;    //开启电机
//    }
//
//
//    if(Key7_Num == 1 && Motor_Start == 1)//不刹车与遥控开启情况由遥控选择模式
//    {
//        Transform = 0;  //关闭科目 //Key7_Num = 1 为手动(遥控，脚踏)
//        if(uart_receiver.channel[3] <= 192)//速度输出设置
//        {
//            ips200_show_string(120, 16*3, "yk");
//            HotRC_control_Speed_duty = 200; //设置档位1
//            sdyk = 0;
//        }
//        else if(uart_receiver.channel[3] == 992)
//        {
//            ips200_show_string(120, 16*3, "Yk");
//            HotRC_control_Speed_duty = 400; //设置档位2
//            sdyk = 0;
//        }
//        else if(uart_receiver.channel[3] == 1792) //遥控换到最大是手动控制
//        {
//            ips200_show_string(120, 16*3, "Sd");
//            sdyk = 1;
//        }
//        else
//        {
//            sdyk = 2;
//            HotRC_control_Speed_duty = 0;
//        }
//        //遥控接收到数据控制车运动
//        if(uart_receiver.channel[1]>1500 && sdyk == 0 )
//        {
//            Back_Motor_SetPwm(HotRC_control_Speed_duty,HotRC_control_Speed_duty);//判断是标志位就给速度
//        }
//        else if(uart_receiver.channel[1]<500  && sdyk == 0 )
//        {
//            Back_Motor_SetPwm(-HotRC_control_Speed_duty,-HotRC_control_Speed_duty);
//        }
//        else if(sdyk == 1)
//        {
//            Manual_Control(Key8_Num);
//        }
//        else
//        {
//            Back_Motor_SetPwm(0,0);
//        }
//
//        //左右控制
//        if(uart_receiver.channel[0]>1500)
//        {
//            Target_Position = 800;//右
//        }
//        else if(uart_receiver.channel[0]<150)
//        {
//            Target_Position = -800;//左
//        }
//        else if(uart_receiver.channel[0]==848)
//        {
//            Target_Position = 0;
//        }
//    }
//    else if(Key7_Num == 0)  //Key7_Num = 0 自动
//    {
//        Transform = 1;  //开启科目
//    }
//
//    if(Motor_Start == 0)
//    {
//        Back_Motor_SetPwm(0,0);  //关闭电机
//    }

}

