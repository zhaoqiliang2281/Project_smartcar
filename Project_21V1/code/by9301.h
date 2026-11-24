/*
 * by9301.h
 *
 *  Created on: 2026年4月15日
 *      Author: bin123
 *  Modified: 模拟 debug UART0 模式，独立使用 UART10 进行 BY9301 MP3 模块通信
 */

#ifndef CODE_BY9301_H_
#define CODE_BY9301_H_

#include "zf_common_typedef.h"
#include "zf_common_fifo.h"

// UART10
#define BY9301_UART_INDEX           (UART_10)            // BY9301 使用的串口索引
#define BY9301_UART_BAUDRATE        (115200)             // 波特率
#define BY9301_UART_TX_PIN          (UART10_TX_P00_8)    // TX 引脚
#define BY9301_UART_RX_PIN          (UART10_RX_P13_1)    // RX 引脚
#define BY9301_UART_USE_INTERRUPT   (1)                  // 是否启用接收中断

#if BY9301_UART_USE_INTERRUPT
#define BY9301_RING_BUFFER_LEN      (128)                // 环形缓冲区大小

void    by9301_interrupt_handler   (void);
uint32  by9301_read_ring_buffer    (uint8 *buff, uint32 len);
#endif

void    by9301_init                 (void);
void    BY9301_Say                  (uint8 NOx);
void    BY9301_setVolume            (uint8 volume);
void    BY9301_playPause            (uint8 playPause);
void    SetBaudRate                 (uint8 BaudRate);

#endif /* CODE_BY9301_H_ */
