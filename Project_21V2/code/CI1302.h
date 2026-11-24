#ifndef CODE_CI1302_H_
#define CODE_CI1302_H_

#include "zf_common_headfile.h"

#define CI1302_UART_INDEX              (UART_9)                       // 默认 UART_2
#define CI1302_UART_BAUDRATE            (9600)                        // 默认 9600
#define CI1302_UART_TX_PIN              (UART9_TX_P20_7)              // 默认 UART2_TX_P10_5
#define CI1302_UART_RX_PIN              (UART9_RX_P20_6)              // 默认 UART2_RX_P10_6


#define FRAME_HEAD_LEN          (4u)        //帧头固定为4字节：0x6B, 0x79, 0x00, 0x81
#define FRAME_TOTAL_LEN         (8u)        //帧结构：帧头(4字节) + 命令码(1字节) + 保留/数据(2字节) + 帧尾(1字节) = 8字节
#define FRAME_TAIL              (0xFBu)     //固定帧尾字节，用于标识一帧数据的结束
#define DATA_STORE_MAX          (64u)       //用于存储历史命令码，最多可存储64条命令

void CI1302_init(void);
void CI1302_process_fifo_data(void);
void CI1302_uart_rx_interrupt_handler(void);

const char *CI1302_get_command_name(uint8 command);
void execute_command(uint8 command);

extern uint8 data_store[DATA_STORE_MAX];
extern uint32 data_store_count;

extern uint8 Sport_sum_flag;     //运动任务数量合计
extern uint8 Flash_sum_flag;     //灯光任务数量合计
extern uint8 Voice_sum_flag;     //鸣笛任务数量合计


#endif /* CODE_CI1302_H_ */
