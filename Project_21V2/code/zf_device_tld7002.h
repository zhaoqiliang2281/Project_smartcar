

#ifndef _zf_device_tld7002_h_
#define _zf_device_tld7002_h_


#include "zf_common_typedef.h"


#define TLD7002_UART_INDEX      (UART_4)
#define TLD7002_UART_BAUD       (2000000)
#define TLD7002_UART_RX         (UART4_TX_P00_9)       //UART1_TX_P20_10
#define TLD7002_UART_HLSIL      (UART4_RX_P00_12)        //UART1_RX_P02_3

#define TLD7002_GPIN0_PIN       (P20_10)                 //P33_4



extern uint16 tld7002_duty[16];

void    tld7002_set_duty        (uint8 tld7002_id);
void    tld7002_callback        (void);
void    tld7002_init            (void);



#endif
