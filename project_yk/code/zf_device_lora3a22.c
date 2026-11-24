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

#include "zf_device_lora3a22.h"

static uint8  lora3a22_uart_data[LORA3A22_DATA_LEN] = {0};         // 接收帧解析缓冲区(仅模块内部使用)

vuint8  lora3a22_finsh_flag = 0;                                    // 表示成功接收到一帧遥控器数据

lora3a22_uart_transfer_dat_struct lora3a22_uart_transfer;

static uint8        lora3a22_uart_fifo_data = 0;                              // 中断中单字节读取缓冲
static uint8        lora3a22_uart_fifo_buffer[LORA3A22_FIFO_SIZE];            // FIFO 数据存放数组

static fifo_struct  lora3a22_uart_fifo;                                       // 定义UART数据FIFO结构体变量
static uint32       lora3a22_fifo_count = 0;                                  // FIFO中当前数据字节数


//--------------------------------  -----------------------------------------------------------------------------------
// 函数简介     lora3a22串口回调函数
// 参数说明     void
// 返回参数     void
// 使用示例     lora3a22_uart_callback();
// 备注信息     此函数需要在串口接收中断内进行调用
//-------------------------------------------------------------------------------------------------------------------
void lora3a22_uart_callback(uint8 uart_dat)
{
    static uint8 length = 0;
    uint8 i = 0;
    uint8 parity_bit_sum  = 0, parity_bit  = 0;

    // 未开始接收时, 必须等到帧头 0xA3 才开始保存数据
    if(0 == length)
    {
        if(LORA3A22_FRAME_STAR != uart_dat)
        {
            return;                                                 // 不是帧头, 丢弃该字节
        }
    }

    lora3a22_uart_data[length++] = uart_dat;

    if(LORA3A22_DATA_LEN <= length)                                 // 收满一帧
    {
        parity_bit = lora3a22_uart_data[1];
        lora3a22_uart_data[1] = 0;
        for(i = 0; i < LORA3A22_DATA_LEN; i ++)
        {
            parity_bit_sum += lora3a22_uart_data[i];
        }

        if (parity_bit_sum == parity_bit)                           // 和校验判断
        {
            lora3a22_uart_data[1]= parity_bit;

            // 模块发送的数据为小端, 与单片机字节序一致, 直接整体拷贝即可
            // 注意: 之前的“大小端交换”是错的, 会导致摇杆值错乱(如中心0、两侧都变负)
            memcpy(((uint8*)&lora3a22_uart_transfer), (uint8*)&lora3a22_uart_data, sizeof(lora3a22_uart_data));

            lora3a22_finsh_flag = 1;
        }
        else
        {
            lora3a22_finsh_flag = 0;
        }

        length = 0;
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     lora3a22初始化函数
// 参数说明     void
// 返回参数     void
// 使用示例     lora3a22_init();
// 备注信息
//-------------------------------------------------------------------------------------------------------------------

void lora3a22_init(void)
{
    fifo_init(&lora3a22_uart_fifo, FIFO_DATA_8BIT, lora3a22_uart_fifo_buffer, LORA3A22_FIFO_SIZE);

    uart_init(LORA3A22_UART_INDEX, LORA3A22_UART_BAUDRATE,LORA3A22_UART_TX_PIN,LORA3A22_UART_RX_PIN );

    uart_rx_interrupt(LORA3A22_UART_INDEX, 1);
}


/**
 * @brief UART接收中断处理函数，将接收的字节写入FIFO
 */
void lora3a22_uart_rx_interrupt_handler(void)
{
    // 一次中断可能积压多个字节, 循环读取直到硬件 FIFO 为空
    while(uart_query_byte(LORA3A22_UART_INDEX, &lora3a22_uart_fifo_data))
    {
        fifo_write_buffer(&lora3a22_uart_fifo, &lora3a22_uart_fifo_data, 1);
    }
}

/**
 * @brief 处理FIFO中待解析的串口数据（应在主循环中调用）
 */
void lora3a22_process_fifo_data(void)
{
    uint8 data = 0;                                         // 使用局部变量, 避免与中断共用 lora3a22_uart_fifo_data
    lora3a22_fifo_count = fifo_used(&lora3a22_uart_fifo);
    while(lora3a22_fifo_count != 0)
    {
        // 逐字节取出 FIFO 中的数据并送入帧解析函数
        if(FIFO_SUCCESS == fifo_read_element(&lora3a22_uart_fifo, &data, FIFO_READ_AND_CLEAN))
        {
            lora3a22_uart_callback(data);
        }
        lora3a22_fifo_count = fifo_used(&lora3a22_uart_fifo);
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     VOFA+ FireWater 协议发送遥控器全部数据
// 参数说明     void
// 返回参数     void
// 使用示例     vofa_firewater_send();
// 备注信息     数据通过调试串口(DEBUG_UART_INDEX)发送
//              FireWater 协议: 逗号分隔的文本, 每帧以换行符 \n 结尾
//              通道顺序: flag, J0~J3, K0~K3, S0~S3 共13个通道
//-------------------------------------------------------------------------------------------------------------------
void vofa_firewater_send(void)
{
    printf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n\r",
           (int)lora3a22_finsh_flag,                                  // 接收完成标志
           (int)lora3a22_uart_transfer.joystick[0],                   // 左摇杆左右
           (int)lora3a22_uart_transfer.joystick[1],                   // 左摇杆上下
           (int)lora3a22_uart_transfer.joystick[2],                   // 右摇杆左右
           (int)lora3a22_uart_transfer.joystick[3],                   // 右摇杆上下
           (int)lora3a22_uart_transfer.key[0],                        // 摇杆左按键
           (int)lora3a22_uart_transfer.key[1],                        // 摇杆右按键
           (int)lora3a22_uart_transfer.key[2],                        // 侧向按键左
           (int)lora3a22_uart_transfer.key[3],                        // 侧向按键右
           (int)lora3a22_uart_transfer.switch_key[0],                 // 左边拨码开关1
           (int)lora3a22_uart_transfer.switch_key[1],                 // 左边拨码开关2
           (int)lora3a22_uart_transfer.switch_key[2],                 // 右边拨码开关1
           (int)lora3a22_uart_transfer.switch_key[3]);                // 右边拨码开关2
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     IPS200 屏幕显示遥控器全部数据
// 参数说明     void
// 返回参数     void
// 使用示例     lora3a22_data_show();
// 备注信息     需要在 ALL_init()(内部已调用 ips200_init) 之后调用
//-------------------------------------------------------------------------------------------------------------------
void lora3a22_data_show(void)
{
    // 接收状态
    if(1 == lora3a22_finsh_flag)
    {
        ips200_show_string(0, 16*0, "Lora OK  ");
    }
    else
    {
        ips200_show_string(0, 16*0, "Lora LOST");
    }

    // 摇杆值
    ips200_show_string(0,   16*1, "J0:");
    ips200_show_string(0,   16*2, "J1:");
    ips200_show_string(0,   16*3, "J2:");
    ips200_show_string(0,   16*4, "J3:");
    ips200_show_int   (32,  16*1, lora3a22_uart_transfer.joystick[0], 6);
    ips200_show_int   (32,  16*2, lora3a22_uart_transfer.joystick[1], 6);
    ips200_show_int   (32,  16*3, lora3a22_uart_transfer.joystick[2], 6);
    ips200_show_int   (32,  16*4, lora3a22_uart_transfer.joystick[3], 6);

    // 按键
    ips200_show_string(0,   16*5, "K:");
    ips200_show_int   (32,  16*5, lora3a22_uart_transfer.key[0], 1);
    ips200_show_int   (48,  16*5, lora3a22_uart_transfer.key[1], 1);
    ips200_show_int   (64,  16*5, lora3a22_uart_transfer.key[2], 1);
    ips200_show_int   (80,  16*5, lora3a22_uart_transfer.key[3], 1);

    // 拨码开关
    ips200_show_string(0,   16*6, "S:");
    ips200_show_int   (32,  16*6, lora3a22_uart_transfer.switch_key[0], 1);
    ips200_show_int   (48,  16*6, lora3a22_uart_transfer.switch_key[1], 1);
    ips200_show_int   (64,  16*6, lora3a22_uart_transfer.switch_key[2], 1);
    ips200_show_int   (80,  16*6, lora3a22_uart_transfer.switch_key[3], 1);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     lora3a22 遥控器控制车运动
// 参数说明     void
// 返回参数     void
// 使用示例     lora3a22_control();
// 备注信息     需先调用 lora3a22_process_fifo_data() 解析数据
//              左摇杆左右 joystick[0] 控制车左右转向
//              右摇杆前后 joystick[3] 控制车前进与后退
//              左右拨码开关 switch_key[0..3] 选择速度档位
//-------------------------------------------------------------------------------------------------------------------
int32 lora3a22_speed_duty = 0;          // 遥控控制电机输出占空比
int32 lora3a22_steer_duty = 90;         // 遥控控制舵机角度

void lora3a22_control(void)
{
    int16 speed_val = lora3a22_uart_transfer.joystick[1];   // 右摇杆前后
    int16 steer_val = lora3a22_uart_transfer.joystick[2];   // 左摇杆左右
    int32 max_duty   = LORA3A22_GEAR_1_DUTY;
    uint8 gear_level = 1;

    // 速度档位选择: 左右拨码开关, 每个开关对应一档, 多个开启时取最高档
    // switch_key[0] -> 1档   switch_key[1] -> 2档
    // switch_key[2] -> 3档   switch_key[3] -> 4档
    if(0 != lora3a22_uart_transfer.switch_key[1]) { gear_level = 2; max_duty = LORA3A22_GEAR_2_DUTY; }
    if(0 != lora3a22_uart_transfer.switch_key[2]) { gear_level = 3; max_duty = LORA3A22_GEAR_3_DUTY; }
    if(0 != lora3a22_uart_transfer.switch_key[3]) { gear_level = 4; max_duty = LORA3A22_GEAR_4_DUTY; }

    // 摇杆限幅, 防止超量程
    if(speed_val > LORA3A22_JOYSTICK_MAX) { speed_val = LORA3A22_JOYSTICK_MAX; }
    if(speed_val < LORA3A22_JOYSTICK_MIN) { speed_val = LORA3A22_JOYSTICK_MIN; }
    if(steer_val > LORA3A22_JOYSTICK_MAX) { steer_val = LORA3A22_JOYSTICK_MAX; }
    if(steer_val < LORA3A22_JOYSTICK_MIN) { steer_val = LORA3A22_JOYSTICK_MIN; }

    // 线性映射: 右摇杆前后 -> 电机占空比 -max_duty ~ +max_duty (正转前进, 负转后退)
    lora3a22_speed_duty = (int32)speed_val * max_duty / LORA3A22_JOYSTICK_MAX;

    // 线性映射: 左摇杆左右 -> 舵机角度 70(左满) ~ 90(中) ~ 110(右满)
    lora3a22_steer_duty = SERVO_MOTOR_MID
                        + (int32)steer_val * (SERVO_MOTOR_R_MAX - SERVO_MOTOR_MID) / LORA3A22_JOYSTICK_MAX;

    // 屏幕显示档位/占空比/编码器
    ips200_show_string(0,   16*8,  "Gear:");
    ips200_show_int   (48,  16*8,  (int32)gear_level, 1);
    ips200_show_string(0,   16*9,  "Spd:");
    ips200_show_int   (48,  16*9,  lora3a22_speed_duty, 5);
    ips200_show_string(0,   16*10, "Ste:");
    ips200_show_int   (48,  16*10, lora3a22_steer_duty, 5);
    ips200_show_string(0,   16*11, "Enc:");
    ips200_show_int   (48,  16*11, encoder[0], 8);

    // 电机与舵机输出
    Pwm_Motor(lora3a22_speed_duty);
    Steer_Set((uint32)lora3a22_steer_duty);
}


