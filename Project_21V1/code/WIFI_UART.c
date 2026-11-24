/*
 * Wifi_uart.c
 *
 *  Created on: 2025年6月16日
 *      Author: liuliangfang
 */

#include "zf_common_headfile.h"

char Fram_Arr[Frame]={0xA1,0xB2,0xC3,0xD4};//发送数据的数组
int16 temp_buffer[Mic_Voice_Arr_Length];
uint8 wifi_uart_get_data=0;//从缓冲区得到的字符串数据
uint8 Command_buffer[10]={0,0,0,0,0};//指令缓存数组
uint8 formol_Command_flag=0;//比赛指令到来标志位
uint8 formol_Command_Num=0;
uint8 nomal_command=0;

uint8 pending_command[MAX_CMD_LEN];   //指令数组
uint8 pending_len = 0;      //收到数据的长度

void WIFI_UART_Init()
{
    while(wifi_uart_init(WIFI_SSID_TEST, WIFI_PASSWORD_TEST, WIFI_UART_STATION)){
        system_delay_ms(500);                                                   // 初始化失败 等待 500ms
    }//连接wifi
    if(1 != WIFI_UART_AUTO_CONNECT){                                             // 如果没有开启自动连接 就需要手动连接目标 IP
        while(wifi_uart_connect_tcp_servers(WIFI_UART_TARGET_IP,WIFI_UART_TARGET_PORT,WIFI_UART_SERIANET))//透传mo's
        {
            system_delay_ms(500);                                               // 建立连接失败 等待 500ms
        }
    }  //跳出循环就是连接成功了
}

//解析数据
void Wifi_Uart_message_parse(void)
{
    if(wifi_uart_read_buffer(&wifi_uart_get_data, 1) != 0) {
        if(wifi_uart_get_data == 170) {
            // 心跳包
            Fram_buffer_Creat(3);
            wifi_uart_send_buffer((uint8 *)Fram_Arr, Frame);
        }
        else {
            if((wifi_uart_get_data >= 1 && wifi_uart_get_data <= 15) ||
               (wifi_uart_get_data >= 30 && wifi_uart_get_data <= 50)) {
                // 比赛指令
                if(formol_Command_flag == 0) {
                    formol_Command_flag = 1;
                    formol_Command_Num = 0;
                    // 清显示
                    ips200_show_string(8*(12), 16*1, "                  ");
                }
                if(formol_Command_Num < 10) {
                    Command_buffer[formol_Command_Num] = wifi_uart_get_data;
                    if(formol_Command_Num <= 6) {
                        ips200_show_uint(8*(12+formol_Command_Num*3), 16*1, Command_buffer[formol_Command_Num], 2);
                    }
                    formol_Command_Num++;
                }
            }
            else if(wifi_uart_get_data >= 16 && wifi_uart_get_data <= 29) {
                // 普通指令
                nomal_command = wifi_uart_get_data;
                ips200_show_string(8*15, 16*0, "   ");
                ips200_show_uint(8*15, 16*0, nomal_command, 2);
            }
            else if(wifi_uart_get_data == 0 && formol_Command_flag == 1) {

                pending_len = (formol_Command_Num > MAX_CMD_LEN) ? MAX_CMD_LEN : formol_Command_Num;
                for (uint8_t i = 0; i < pending_len; i++) {
                    pending_command[i] = Command_buffer[i];
                }
                // 清空接收缓冲区
                formol_Command_flag = 0;
                formol_Command_Num = 0;
                memset(Command_buffer, 0, 10);
            }

        }
    }
}


void Fram_buffer_Creat(uint8 Flag)
{
    switch (Flag)
   {
        case 3://心跳包
            Fram_Arr[4]=0x03;
            memset(&Fram_Arr[5],0xDD,640);
            break;
        default://数据包
            Fram_Arr[4]=Flag;
            switch(Position){
                case Zero:
                    memcpy(&Fram_Arr[5],&Mic_Voice[One][0],640);
                    break;
                case One:
                    memcpy(&Fram_Arr[5],&Mic_Voice[Zero][0],640);
                    break;
            }
            break;
    }
}

//发送音频数据包
void Wifi_Uart_Continue_data_Send()//中间数据的发送
{
        Voice_Get_Flag=0;
        Fram_buffer_Creat((uint8)1);
        wifi_uart_send_buffer((uint8 *)Fram_Arr,Frame);
}
