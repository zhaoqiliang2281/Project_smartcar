/*
 * Common_peripheral.h
 *
 *  Created on: 2025年6月12日
 *      Author: 吉祥止止
 */

#ifndef CODE_COMMON_PERIPHERAL_H_
#define CODE_COMMON_PERIPHERAL_H_

#define IPS200_TYPE (IPS200_TYPE_SPI)

//宏定义配置引脚
#define BUZZER_PIN  (P33_10)

#define LED1                    (P20_9)
#define LED2                    (P20_8)
#define LED3                    (P21_5)
#define LED4                    (P21_4)

#define KEY1                    (P20_6)
#define KEY2                    (P20_7)
#define KEY3                    (P11_2)
#define KEY4                    (P11_3)
#define KEY5                    (P21_7)

#define sw1                     (P33_11)
#define sw2                     (P33_12)

extern uint8 key1_flag;
extern uint8 key2_flag;
extern uint8 key3_flag;
extern uint8 key4_flag;
extern uint8 key5_flag;

extern uint8_t delay_Start;   // 延迟开启
extern uint8 delay_ScanTemp;

void Buzzer_check(uint16 time);

void Buzzer_SetTimer(uint16 Count);
void Buzzer_scan(void);

uint8 Timer_delay_ms(uint16 time);
void Timer_delay_scan(void);

void key_scan(void);

void LED_init(void);

void ALL_init(void);

#endif /* CODE_COMMON_PERIPHERAL_H_ */
