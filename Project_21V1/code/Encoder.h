/*
 * Encoder.h
 *
 *  Created on: 2026年3月7日
 *      Author: LiangZhaoqi
 */
#ifndef CODE_ENCODER_H_
#define CODE_ENCODER_H_

#include "zf_common_headfile.h"

//后轮编码器
#define ENCODER_TIM2_A                 (TIM2_ENCODER_CH1_P33_7)
#define ENCODER_TIM2_B                 (TIM2_ENCODER_CH2_P33_6)//右

#define ENCODER_TIM5_A                 (TIM5_ENCODER_CH1_P10_3)
#define ENCODER_TIM5_B                 (TIM5_ENCODER_CH2_P10_1)//左


void Encoder_Init(encoder_index_enum ENCODER);
int16 Read_Encoder(encoder_index_enum ENCODER);
void Clear_Encoder(encoder_index_enum ENCODER);
void Read_Encoder_Value(encoder_index_enum ENCODER,int16 *Encoder_Value,int16 *Encoder_Aggregate_Value);

void Read_Encoder_Value_Update(encoder_index_enum ENCODER);
int16 Unit_Time_Read_Encoder_Value(encoder_index_enum ENCODER);
int32 Read_Encoder_Aggregate_Value(encoder_index_enum ENCODER);

#endif /* CODE_ENCODER_H_ */
