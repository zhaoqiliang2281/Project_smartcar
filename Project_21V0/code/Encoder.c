/*
 * Encoder.c
 *
 *  Created on: 2026年3月5日
 *      Author: liang2281
 */

#include "Encoder.h"
#include <math.h>
#include <stdint.h>

/************************************************
 *  函数名称: 编码器初始化：void Encoder_Init(encoder_index_enum ENCODER)
 *  功能说明：
 *  参数说明：
 *  参   数：编码器枚举
 *  函数返回：无
 *  修改时间：2026年3月5日
 *  备   注：
 ************************************************/
void Encoder_Init(encoder_index_enum ENCODER)
{
    switch(ENCODER)
    {
        case TIM2_ENCODER:
        {
            encoder_dir_init(TIM2_ENCODER, ENCODER_TIM2_A, ENCODER_TIM2_B);
            Clear_Encoder(TIM2_ENCODER);
        }break;

        case TIM4_ENCODER:
        {
            encoder_quad_init(TIM4_ENCODER, ENCODER_TIM4_A, ENCODER_TIM4_B);
            Clear_Encoder(TIM4_ENCODER);
        }break;

        case TIM5_ENCODER:
        {
            encoder_dir_init(TIM5_ENCODER, ENCODER_TIM5_A, ENCODER_TIM5_B);
            Clear_Encoder(TIM5_ENCODER);
        }break;

        default: break;
    }
}

/************************************************
 *  函数名称: 清空编码值：void Clear_Encoder(encoder_index_enum ENCODER)
 *  功能说明：
 *  参数说明：
 *  参   数：编码器枚举
 *  函数返回：无
 *  修改时间：2025年3月5日
 *  备   注：
 ************************************************/
void Clear_Encoder(encoder_index_enum ENCODER)
{
    encoder_clear_count(ENCODER);
}

/************************************************
 *  函数名称: 编码器读取：int16 Read_Encoder(encoder_index_enum ENCODER)
 *  功能说明：
 *  参数说明：
 *  参   数：编码器枚举
 *  函数返回：调用时间内的编码值
 *  修改时间：2025年3月5日
 *  备   注：读取并清空编码计数值
 ************************************************/
int16 Read_Encoder(encoder_index_enum ENCODER)
{
    int16 Temp=0;

    Temp = encoder_get_count(ENCODER);      // 获取编码器计数
    encoder_clear_count(ENCODER);//清除编码器cnt值

    return Temp;
}

/************************************************
 *  函数名称: 编码器读取：void Read_Encoder_Value(encoder_index_enum ENCODER,int16 *Encoder_Value,int16 *Encoder_Aggregate_Value)
 *  功能说明：
 *  参数说明：
 *  参   数： 编码器枚举
 *          调用时间内的编码值
 *  函数返回： 累计编码值
 *  修改时间：2025年3月5日
 *  备   注：同一编码器不能与Read_Encoder函数同时使用
 ************************************************/
void Read_Encoder_Value(encoder_index_enum ENCODER,int16 *Encoder_Value,int16 *Encoder_Aggregate_Value)
{
    *Encoder_Value = encoder_get_count(ENCODER);      // 获取编码器计数

    Encoder_Aggregate_Value += *Encoder_Value;

    encoder_clear_count(ENCODER);//清除编码器cnt值
}


