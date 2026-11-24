/*
 * Algorithmic_Filtering.h
 *
 *  Created on: 2026年3月24日
 *      Author: LiangZhaoqi
 */

#ifndef CODE_ALGORITHMIC_FILTERING_H_
#define CODE_ALGORITHMIC_FILTERING_H_

#include "zf_common_headfile.h"

extern uint8 delay_ScanTemp;    //定时器延迟完成标志位
extern uint8 Motor_Close_Temp;  //清除由踩油门由于一阶低通滤波的影响
extern uint8 Motor_Start;       //电机开启标志位
extern uint8 Transform;         //科目与手动开转换标志位


uint16_t map_voltage_adc(uint16_t adc_val);
int16 Manual_Control(uint8 flag);
void Auto_control(void);

#define FILTER_WINDOW_SIZE 10  // 滤波窗口大小

// 定义滤波结构体
typedef struct
{
    int32 buffer[FILTER_WINDOW_SIZE]; // 数据缓冲区 (根据传感器精度选择 int16_t 或 int32_t)
    uint8 index;                      // 当前写入位置索引
    int32 sum;                        // 当前窗口数据的总和 (用于快速计算，避免每次遍历累加)
    uint8 is_full;                    // 标记缓冲区是否已填满
} MeanFilter_TypeDef;

// 卡尔曼滤波结构体 (针对速度估计)
typedef struct {
    float x;      // 状态估计值 (例如：速度)
    float p;      // 估计误差协方差 (不确定性)
    float q;      // 过程噪声协方差 (模型有多不准)
    float r;      // 测量噪声协方差 (传感器有多噪)
    float k;      // 卡尔曼增益 (自动计算)
} KalmanFilter;



extern MeanFilter_TypeDef adc_filter;
extern KalmanFilter Kalman_Filter;

uint16_t map_voltage_adc(uint16_t adc_val);

int32 Filter(int32 rawValue);
void MeanFilter_Init(MeanFilter_TypeDef *filter);
int32_t MeanFilter_Update(MeanFilter_TypeDef *filter, int32 new_data);

void Kalman_Init(KalmanFilter *kf, float init_val, float q, float r);
float Kalman_Update(KalmanFilter *kf, float measurement);



#endif /* CODE_ALGORITHMIC_FILTERING_H_ */
