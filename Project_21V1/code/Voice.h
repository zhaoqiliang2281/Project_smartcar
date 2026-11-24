/*
 * Voice.h
 *
 *  Created on: 2026年4月1日
 *      Author: liuliangfang
 */

#ifndef CODE_VOICE_H_
#define CODE_VOICE_H_

#define Mic1            (ADC0_CH0_A0)
#define Mic2            (ADC0_CH1_A1)
#define Resolution      (ADC_12BIT)
#define Mic_Voice_Arr_Length    (320)

typedef enum
{
    Zero=0,
    One=1
}Position_enum;

// 外部变量声明
extern int16 Mic_Voice[2][Mic_Voice_Arr_Length];
extern uint16 Voice_Arr_Position;
extern Position_enum Position;
extern uint8 Voice_Get_Flag;

// 函数声明
void Voice_Init(void);
void Voice_Get(void);

#endif /* CODE_VOICE_H_ */
