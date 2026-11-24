/*
 * Common_peripheral.h
 *
 *  Created on: 2025年6月12日
 *      Author: LiangZhaoqi
 */

#ifndef CODE_COMMON_PERIPHERAL_H_
#define CODE_COMMON_PERIPHERAL_H_

#define IPS200_TYPE (IPS200_TYPE_SPI)

//宏定义配置引脚
#define BUZZER_PIN  (P33_8)        //旧版板子引脚P33_10

#define LED1                    (P20_9)//
#define LED2                    (P20_9)     //P20_8
#define LED3                    (P21_5)
#define LED4                    (P21_4)

#define KEY4                    (P33_13)     //P20_6
#define KEY3                    (P02_8)     //P20_7
#define KEY2                    (P20_0)     //P11_2
#define KEY1                    (P33_4)     //P11_3

#define KEY5                    (P21_7)

#define KEY9                    (P11_11)    //旋转编码器按键

//#define KEY6                    (P33_5)     //脚踏
//#define KEY7                    (P20_0)     //自动与手动切换
//#define KEY8                    (P20_3)     //手动情况下前后方向切换

#define KEY6                    (P10_2)     //脚踏
#define KEY7                    (P33_5)     //自动与手动切换
#define KEY8                    (P20_3)     //手动情况下前后方向切换


//#define KEY6                    (P10_2)     //脚踏
//#define KEY7                    (P20_3)     //自动与手动切换
//#define KEY8                    (P33_5)     //手动情况下前后方向切换

#define sw1                     (P11_9)    //P33_11
#define sw2                     (P11_10)    //P33_12

#define VOLTAGE_PORT     ADC0_CH2_A2    // 定义ADC采集端口ADC0_CH0_A0//油门

extern uint8 key1_flag;
extern uint8 key2_flag;
extern uint8 key3_flag;
extern uint8 key4_flag;
extern uint8 key5_flag;

extern uint8 key9_flag;

extern uint8 key1_state;
extern uint8 key2_state;
extern uint8 key3_state;
extern uint8 key4_state;
extern uint8 key5_state;

extern uint8 key9_flag;

extern uint8 key1_state_last;
extern uint8 key2_state_last;
extern uint8 key3_state_last;
extern uint8 key4_state_last;
extern uint8 key5_state_last;

extern uint8 key9_state_last;

extern uint8_t delay_Start;   // 延迟开启标志位
extern uint8 delay_ScanTemp;  // 定时器到达时间置1

void Buzzer_init(void);
void Buzzer_check(uint16 time);

void Buzzer_SetTimer(uint16 Count);
void Buzzer_scan(void);

uint8 Timer_delay_ms(uint16 time);
void Timer_delay_scan(void);

void Key_init(void);
void key_Scan(void);

void LED_init(void);
void LED_Tick(void);

float main_loop_freq_update(void);

void ALL_init(void);

#endif /* CODE_COMMON_PERIPHERAL_H_ */
