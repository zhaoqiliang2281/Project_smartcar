/*
 * Wifi_uart.h
 *
 *  Created on: 2025年6月16日
 *      Author: liuliangfang
 */

#ifndef CODE_WIFI_UART_H_
#define CODE_WIFI_UART_H_

#define Frame     (645)//四字节帧头＋一字节标志位+1280长度数据
#define Frame_Header (0xA1,0xB2,0xC3,0xD4)
#define WIFI_SSID_TEST          "blunt"
#define WIFI_PASSWORD_TEST      "99999999"
#define MAX_CMD_LEN 5           //接收的数据个数

extern char Fram_Arr[Frame];//发送数据的数组
extern uint8 nomal_command;
extern uint8 Command_buffer[10];//指令缓存数组
extern uint8 pending_command[MAX_CMD_LEN];//保存的命令数据
extern uint8 formol_Command_flag;//比赛指令到来标志位
extern uint8 formol_Command_Num;
void WIFI_UART_Init(void);
void Wifi_Uart_message_parse(void);
void Fram_buffer_Creat(uint8 Flag);
void Wifi_Uart_Continue_data_Send(void);//中间数据的发送


extern uint8 pending_len;

#endif /* CODE_WIFI_UART_H_ */
