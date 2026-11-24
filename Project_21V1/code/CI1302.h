#ifndef CODE_CI1302_H_
#define CODE_CI1302_H_

#include "zf_common_typedef.h"

#define UART2_INDEX_CI1302              (UART_9)                           // Ĭ�� UART_2
#define UART2_BAUDRATE_CI1302            (9600)                           // Ĭ�� 9600
#define UART2_TX_PIN_CI1302              (UART9_TX_P20_7)                           // Ĭ�� UART2_TX_P10_5
#define UART2_RX_PIN_CI1302              (UART9_RX_P20_6)                           // Ĭ�� UART2_RX_P10_6


#define FRAME_HEAD_LEN          (4u)        //֡ͷ�̶�Ϊ4�ֽڣ�0x6B, 0x79, 0x00, 0x81
#define FRAME_TOTAL_LEN         (8u)        //֡�ṹ��֡ͷ(4�ֽ�) + ������(1�ֽ�) + ����/����(2�ֽ�) + ֡β(1�ֽ�) = 8�ֽ�
#define FRAME_TAIL              (0xFBu)     //�̶�֡β�ֽڣ����ڱ�ʶһ֡���ݵĽ���
#define DATA_STORE_MAX          (64u)       //���ڴ洢��ʷ�����룬���ɴ洢64������

void CI1302_init(uint8 uart_index, uint32 baudrate, uint32 uart_tx_pin, uint32 uart_rx_pin);
void CI1302_process_fifo_data(void);
void CI1302_uart_rx_interrupt_handler(void);
const char *CI1302_get_command_name(uint8 command);
void execute_command(uint8 command);

void CI1302_uart_rx_interrupt_handler(void);

extern uint8 data_store[DATA_STORE_MAX];
extern uint32 data_store_count;

extern uint8 Sport_sum_flag;     //�˶����������ϼ�
extern uint8 Flash_sum_flag;     //�ƹ����������ϼ�
extern uint8 Voice_sum_flag;     //�������������ϼ�
extern uint8 Chinese_task_display; //����������ʾ����

#endif /* CODE_CI1302_H_ */
