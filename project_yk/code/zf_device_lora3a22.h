/*********************************************************************************************************************
* TC264 Opensourec Library 即（TC264 开源库）是一个基于官方 SDK 接口的第三方开源库
* Copyright (c) 2022 SEEKFREE 逐飞科技
*
* 本文件是 TC264 开源库的一部分
*
* TC264 开源库 是免费软件
* 您可以根据自由软件基金会发布的 GPL（GNU General Public License，即 GNU通用公共许可证）的条款
* 即 GPL 的第3版（即 GPL3.0）或（您选择的）任何后来的版本，重新发布和/或修改它
*
* 本开源库的发布是希望它能发挥作用，但并未对其作任何的保证
* 甚至没有隐含的适销性或适合特定用途的保证
* 更多细节请参见 GPL
*
* 您应该在收到本开源库的同时收到一份 GPL 的副本
* 如果没有，请参阅<https://www.gnu.org/licenses/>
*
* 额外注明：
* 本开源库使用 GPL3.0 开源许可证协议 以上许可申明为译文版本
* 许可申明英文版在 libraries/doc 文件夹下的 GPL3_permission_statement.txt 文件中
* 许可证副本在 libraries 文件夹下 即该文件夹下的 LICENSE 文件
* 欢迎各位使用并传播本程序 但修改内容时必须保留逐飞科技的版权声明（即本声明）
*
* 文件名称          zf_device_lora3a22
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          ADS v1.9.4
* 适用平台          TC264D
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2024-03-29       JKS            first version
********************************************************************************************************************/


#ifndef CODE_ZF_DEVICE_LORA3A22_H_
#define CODE_ZF_DEVICE_LORA3A22_H_

#include "zf_common_headfile.h"


#define LORA3A22_UART_INDEX            (UART_2)              // 定义串口遥控器使用的串口
#define LORA3A22_UART_TX_PIN           (UART2_TX_P10_5)        // 遥控器接收机的RX引脚 连接单片机的TX引脚
#define LORA3A22_UART_RX_PIN           (UART2_RX_P10_6)        // 遥控器接收机的TX引脚 连接单片机的RX引脚
#define LORA3A22_UART_BAUDRATE         (115200)              // 指定 lora3a22 串口所使用的的串口波特率

#define LORA3A22_DATA_LEN              ( 18  )               // lora3a22帧长
#define LORA3A22_FRAME_STAR            ( 0XA3 )              // 帧头信息
#define LORA3A22_FIFO_SIZE             ( 128 )               // lora3a22接收FIFO缓冲区大小

// 摇杆量程 (根据实测摇杆最大值/最小值调整)
#define LORA3A22_JOYSTICK_MAX          ( 2028 )              // 摇杆正向最大量程
#define LORA3A22_JOYSTICK_MIN          (-1929 )              // 摇杆反向最大量程

// 速度档位 (电机占空比百分比, Pwm_Motor 范围为 -100~100)
#define LORA3A22_GEAR_1_DUTY           ( 10 )                // 1档 速度
#define LORA3A22_GEAR_2_DUTY           ( 15 )                // 2档 速度
#define LORA3A22_GEAR_3_DUTY           ( 20 )                // 3档 速度
#define LORA3A22_GEAR_4_DUTY           ( 25 )                // 4档 速度


typedef struct
{
    uint8 head;                         // 帧头
    uint8 sum_check;                    // 和校验

    int16 joystick[4];
	// joystick[0]:左边摇杆左右值
	// joystick[1]:左边摇杆上下值
	// joystick[2]:右边摇杆左右值
	// joystick[3]:右边摇杆上下值

    uint8 key[4];
	// 按下1 松开0
    // key[0]-摇杆左边
    // key[1]-摇杆右边
    // key[2]-侧向按键左边
    // key[3]-侧向按键右边

    uint8 switch_key[4];
    // switch_key[0]-左边拨码开关_1
    // switch_key[1]-左边拨码开关_2
    // switch_key[2]-右边拨码开关_1
    // switch_key[3]-右边拨码开关_2

}lora3a22_uart_transfer_dat_struct;


extern lora3a22_uart_transfer_dat_struct lora3a22_uart_transfer;
extern vuint8  lora3a22_finsh_flag;


void lora3a22_uart_callback(uint8 uart_dat);

void lora3a22_init(void);


void lora3a22_uart_rx_interrupt_handler(void);
void lora3a22_process_fifo_data(void);


void vofa_firewater_send(void);
void lora3a22_data_show(void);
void lora3a22_control(void);


#endif
