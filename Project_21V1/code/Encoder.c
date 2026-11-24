/*
 * Encoder.c
 *
 *  Created on: 2026年3月5日
 *      Author: LiangZhaoqi
 */

#include "Encoder.h"
#include <math.h>
#include <stdint.h>

//左电机编码值变量
static int16 Left_Encoder_Value = 0;             //单位时间读取到编码值
static int32 Left_Encoder_Aggregate_Value = 0;   //读取编码累计值

//右电机编码值变量
static int16 Right_Encoder_Value = 0;             //单位时间读取到编码值
static int32 Right_Encoder_Aggregate_Value = 0;   //读取编码累计值

/************************************************
 *  函数名称: 编码器初始化
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

        case TIM5_ENCODER:
        {
            encoder_dir_init(TIM5_ENCODER, ENCODER_TIM5_A, ENCODER_TIM5_B);
            Clear_Encoder(TIM5_ENCODER);
        }break;

        default: break;
    }
}

/************************************************
 *  函数名称: 清空编码值
 *  功能说明：
 *  参数说明：
 *  参   数：编码器枚举
 *  函数返回：无
 *  修改时间：2026年3月5日
 *  备   注：
 ************************************************/
void Clear_Encoder(encoder_index_enum ENCODER)
{
    encoder_clear_count(ENCODER);
}

/************************************************
 *  函数名称: 编码器读取
 *  功能说明：
 *  参数说明：编码器枚举 TIM2_ENCODER:右 TIM5_ENCODER:左
 *  参   数：编码器枚举
 *  函数返回：调用时间内的编码值
 *  修改时间：2026年3月5日
 *  备   注：读取并清空编码计数值
 ************************************************/
int16 Read_Encoder(encoder_index_enum ENCODER)
{
    int16 Temp=0;

    Temp = encoder_get_count(ENCODER);      // 获取编码器计数
    encoder_clear_count(ENCODER);           //清除编码器cnt值

    return Temp;
}

/************************************************
 *  函数名称: 编码器读取
 *  功能说明：
 *  参数说明：编码器枚举 TIM2_ENCODER:右 TIM5_ENCODER:左
 *  参   数： 编码器枚举
 *          调用时间内的编码值
 *          累计编码值
 *  函数返回：无
 *  修改时间：2026年3月5日
 *  备   注：同一时间同一编码器不能与Read_Encoder函数同时使用
 ************************************************/
void Read_Encoder_Value(encoder_index_enum ENCODER,int16 *Encoder_Value,int16 *Encoder_Aggregate_Value)
{
    *Encoder_Value = encoder_get_count(ENCODER);       // 获取编码器计数

    *Encoder_Aggregate_Value += *Encoder_Value;        //获取累积编码值

    encoder_clear_count(ENCODER);                      //清除编码器cnt值
}

/************************************************
 *  函数名称: 编码器读取单位时间内的编码值与累计值
 *  功能说明：
 *  参数说明：TIM2_ENCODER:右 TIM5_ENCODER:左
 *  参   数：编码器枚举
 *  函数返回：无
 *  修改时间：2026年4月30日
 *  备   注：放在中断调用：时间间隔：20ms
 ************************************************/
void Read_Encoder_Value_Update(encoder_index_enum ENCODER)
{
    switch (ENCODER)
    {
        case TIM2_ENCODER:
        {
            Right_Encoder_Value = encoder_get_count(TIM2_ENCODER);          // 获取编码器计数
            Right_Encoder_Aggregate_Value += Right_Encoder_Value;           //获取累积编码值
            encoder_clear_count(TIM2_ENCODER);                              //清除编码器cnt值
        }
            break;
        case TIM5_ENCODER:
        {
            Left_Encoder_Value = encoder_get_count(TIM5_ENCODER);           // 获取编码器计数
            Left_Encoder_Aggregate_Value += Left_Encoder_Value;             //获取累积编码值
            encoder_clear_count(TIM5_ENCODER);                              //清除编码器cnt值
        }
            break;
        default: break;
    }

}
/************************************************
 *  函数名称: 编码器读取调用时间内的编码值
 *  功能说明：
 *  参数说明：TIM2_ENCODER:右 TIM5_ENCODER:左
 *  参   数： 编码器枚举
 *  函数返回：调用时间内的编码值
 *  修改时间：2026年4月30日
 *  备   注：
 ************************************************/
int16 Unit_Time_Read_Encoder_Value(encoder_index_enum ENCODER)
{
    int16 Encoder_Value = 0;
    switch (ENCODER)
    {
        case TIM2_ENCODER:
        {
            Encoder_Value = -Right_Encoder_Value;
        }
            break;
        case TIM5_ENCODER:
        {
            Encoder_Value = Left_Encoder_Value;
        }
            break;
        default: break;
    }
    return Encoder_Value;
}

/************************************************
 *  函数名称: 编码器读取累计编码值
 *  功能说明：
 *  参数说明：TIM2_ENCODER:右 TIM5_ENCODER:左
 *  参   数：编码器枚举
 *  函数返回：累计编码值
 *  修改时间：2026年4月30日
 *  备   注：
 ************************************************/
int32 Read_Encoder_Aggregate_Value(encoder_index_enum ENCODER)
{
    int32 Encoder_Aggregate_Value = 0;
    switch (ENCODER)
    {
        case TIM2_ENCODER:  //右
        {
            Encoder_Aggregate_Value = -Right_Encoder_Aggregate_Value;
        }
            break;
        case TIM5_ENCODER:  //左
        {
            Encoder_Aggregate_Value = Left_Encoder_Aggregate_Value;
        }
            break;
        default: break;
    }
    return Encoder_Aggregate_Value;
}
