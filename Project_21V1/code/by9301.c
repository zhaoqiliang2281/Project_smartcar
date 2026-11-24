/*
 * by9301.c
 *
 *  Created on: 2026年4月15日
 *      Author: bin123
 *  Modified: 模拟 debug UART0 的模式，独立初始化 UART10 进行 BY9301 MP3 模块通信
 *            不再占用 debug_init()，使用独立的 by9301_init()
 */

#include "zf_common_headfile.h"
#include "by9301.h"

static volatile uint8 by9301_init_flag = 0;     // 初始化标志

#if BY9301_UART_USE_INTERRUPT
uint8               by9301_uart_buffer[BY9301_RING_BUFFER_LEN];   // 接收数据环形缓冲区
uint8               by9301_uart_data;                              // 单字节接收暂存
fifo_struct         by9301_uart_fifo;                              // FIFO 结构体
#endif

//-------------------------------------------------------------------------------------------------------------------
// 函数说明     BY9301 串口初始化（模拟 debug_init 模式）
// 参数说明     void
// 返回参数     void
// 使用示例     by9301_init();
// 备注信息     独立初始化 UART10，不与 debug UART0 冲突
//-------------------------------------------------------------------------------------------------------------------
void by9301_init (void)
{
    uart_init(BY9301_UART_INDEX,
              BY9301_UART_BAUDRATE,
              BY9301_UART_TX_PIN,
              BY9301_UART_RX_PIN);

#if BY9301_UART_USE_INTERRUPT
    fifo_init(&by9301_uart_fifo, FIFO_DATA_8BIT, by9301_uart_buffer, BY9301_RING_BUFFER_LEN);
    uart_rx_interrupt(BY9301_UART_INDEX, 1);                        // 使能 UART10 接收中断
#endif

    by9301_init_flag = 1;
}

#if BY9301_UART_USE_INTERRUPT

//-------------------------------------------------------------------------------------------------------------------
// 函数说明     BY9301 接收中断处理函数（模拟 debug_interrupr_handler）
//             在 isr.c 的 uart10_rx_isr 中调用
// 参数说明     void
// 返回参数     void
// 使用示例     by9301_interrupt_handler();
// 备注信息     将接收到的数据写入环形缓冲区 FIFO
//-------------------------------------------------------------------------------------------------------------------
void by9301_interrupt_handler (void)
{
    if(by9301_init_flag)
    {
        uart_query_byte(BY9301_UART_INDEX, &by9301_uart_data);      // 读取接收字节
        fifo_write_buffer(&by9301_uart_fifo, &by9301_uart_data, 1); // 写入 FIFO
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数说明     读取 BY9301 环形缓冲区数据（模拟 debug_read_ring_buffer）
// 参数说明     *buff       存放数据的数组指针
// 参数说明     len         需要读取的长度
// 返回参数     uint32      实际读取到的数据长度
// 使用示例     by9301_read_ring_buffer(buf, 64);
// 备注信息     如果 BY9301 有返回数据，可通过此函数读取
//-------------------------------------------------------------------------------------------------------------------
uint32 by9301_read_ring_buffer (uint8 *buff, uint32 len)
{
    fifo_read_buffer(&by9301_uart_fifo, buff, &len, FIFO_READ_AND_CLEAN);
    return len;
}

#endif

/**
  * @brief  指定曲目播放
  * @param  NOx: mp3文件序号
  * @retval None
  */
void BY9301_Say(uint8 NOx)
{
    uint8 arr[8] = {0};
    arr[0] = 0x7E;
    arr[1] = 0x05;
    arr[2] = 0x41;
    arr[3] = 0x00;
    arr[4] = NOx;
    arr[5] = arr[1]^arr[2]^arr[3]^arr[4];
    arr[6] = 0xEF;
    uart_write_buffer(BY9301_UART_INDEX, arr, 7);
}

/**
  * @brief  设置音量
  * @param  volume: 0-30
  * @retval None
  */
void BY9301_setVolume(uint8 volume)
{
    if(volume > 30) volume = 30;
    uint8 arr[7];
    arr[0] = 0x7E;
    arr[1] = 0x04;
    arr[2] = 0x31;
    arr[3] = volume;
    arr[4] = arr[1]^arr[2]^arr[3];
    arr[5] = 0xEF;
    uart_write_buffer(BY9301_UART_INDEX, arr, 6);
}

/**
  * @brief  设置播放/暂停
  * @param  playPause: 0:pause 1:play
  * @retval None
  */
void BY9301_playPause(uint8 playPause)
{
    uint8 arr[6];
    arr[0] = 0x7E;
    arr[1] = 0x03;
    if(playPause) {
        arr[2] = 0x01;
    } else {
        arr[2] = 0x02;
    }
    arr[3] = arr[1]^arr[2];
    arr[4] = 0xEF;
    uart_write_buffer(BY9301_UART_INDEX, arr, 5);
}

/*************************************************************
 ** 函 数 名:   void SetBaudRate(uint8_t BaudRate)
 ** 说    明:   设置波特率
 ** 输入参数:   要设置的波特率: 00: 9600 04: 115200
 ** 输出参数:   无
 ** 返 回 值:   无
 ** 备    注:   无
 *************************************************************/
void SetBaudRate(uint8 BaudRate)
{
    uint8 arr[7];
    arr[0] = 0x7E;
    arr[1] = 0x04;
    arr[2] = 0x36;
    arr[3] = BaudRate;
    arr[4] = arr[1]^arr[2]^arr[3];
    arr[5] = 0xEF;
    uart_write_buffer(BY9301_UART_INDEX, arr, 6);
}
