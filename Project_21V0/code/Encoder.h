/*
 * Encoder.h
 *
 *  Created on: 2026年3月7日
 *      Author: liang2281
 */

/*
//剩下编码器对应定时器与接口
#define ENCODER_3                      (TIM6_ENCODER)
#define ENCODER_TIM6_A                 (TIM6_ENCODER_CH1_P20_3)
#define ENCODER_TIM6_B                 (TIM6_ENCODER_CH2_P20_0)
*/


#ifndef CODE_ENCODER_H_
#define CODE_ENCODER_H_


#include "zf_common_headfile.h"

//前轮编码器
#define ENCODER_TIM4_A                 (TIM4_ENCODER_CH1_P02_8)//b
#define ENCODER_TIM4_B                 (TIM4_ENCODER_CH2_P00_9)//a

//后轮编码器
#define ENCODER_TIM2_A                 (TIM2_ENCODER_CH1_P33_7)
#define ENCODER_TIM2_B                 (TIM2_ENCODER_CH2_P33_6)//

#define ENCODER_TIM5_A                 (TIM5_ENCODER_CH1_P10_3)
#define ENCODER_TIM5_B                 (TIM5_ENCODER_CH2_P10_1)//


void Encoder_Init(encoder_index_enum ENCODER);
int16 Read_Encoder(encoder_index_enum ENCODER);
void Clear_Encoder(encoder_index_enum ENCODER);
void Read_Encoder_Value(encoder_index_enum ENCODER,int16 *Encoder_Value,int16 *Encoder_Aggregate_Value);

#endif /* CODE_ENCODER_H_ */
