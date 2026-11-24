/*
 * Algorithmic_Filtering.c
 *
 *  Created on: 2026年3月24日
 *      Author: LiangZhaoqi
 */

#include "Algorithmic_Filtering.h"

extern uint8 dispatch_car; //发车标志位(main定义，发车按键按下，1秒后发车，科目完成后标志位清0)

// static uint8_t Key6_Num,Key7_Num,Key8_Num;
uint8 Transform = 0;    //科目与手动转换标志位
uint8 Motor_Start = 0;  //电机开启标志位
uint8 Motor_Close_Temp = 0;//清除由低通滤波产生的滞后影响标志位

/************************************************
 *  函数名称: 值线性映射
 *  功能说明：将 ADC 值线性映射到 OUT_MIN - OUT_MAX
 *  参数说明：adc_val 原始 ADC 值 (IN_MIN - IN_MAX)
 *  参   数：原始 ADC 值
 *  函数返回：将 ADC 值线性映射值
 *  修改时间：2026年3月8日
 *  备   注：这里输入与输出范围已经根据当前车的脚踏进行限幅与范围调整
 ************************************************/
uint16_t map_voltage_adc(uint16_t adc_val)
{
    const int16_t IN_MIN  = 2040;
    const int16_t IN_MAX  = 4030;
    const int16_t OUT_MIN = 0;
    const int16_t OUT_MAX = 1000;

    if (adc_val <= IN_MIN) return OUT_MIN;
    if (adc_val >= IN_MAX) return OUT_MAX;

    int32_t numerator = ((int32_t)adc_val - IN_MIN) * (OUT_MAX - OUT_MIN);
    int32_t denominator = (IN_MAX - IN_MIN); // 1800

    int32_t result = (numerator + (denominator / 2)) / denominator;

    return (uint16_t)result;
}

/************************************************
 *  函数名称：脚踏控制车运动
 *  功能说明：采集脚踏传感器ADC值与滤波，并进行一阶低通滤波
 *  参数说明：调节参数 α 打开注释 printf 上位机看值
 *  参   数：Dir是电机方向选择，0：正方向，1：反方向
 *  函数返回：无
 *  修改时间：2026年3月13日
 *  备    注：根据实际情况修改参数，目前加速反应快，减速反应慢，避免减速反应过快像刹车
 ************************************************/
int16 Manual_Control(uint8 Dir)
{
    uint16 valtage_adc_Value = 0;
    uint16 valtage_target_Value = 0;
    static uint16_t valtage_filtered = 0;
    uint16 valtage_now_Value = 0;
    float current_alpha = 0.0f;
    int16 Motor_Pwm = 0;

    const float alpha_fast = 0.2f;
    const float alpha_slow = 0.025f;

    valtage_adc_Value = adc_mean_filter_convert(VOLTAGE_PORT,5);

    valtage_target_Value = map_voltage_adc(valtage_adc_Value);

    if(valtage_target_Value < 50){valtage_target_Value = 0;}

    if (valtage_target_Value >= valtage_filtered)
    {
        current_alpha = alpha_fast;
    }
    else
    {
        current_alpha = alpha_slow;
    }

    valtage_now_Value = (uint16_t)(valtage_target_Value * current_alpha + valtage_filtered * (1.0f - current_alpha));
    valtage_filtered = valtage_now_Value;

    if (valtage_target_Value == 0 && valtage_now_Value < 2) {valtage_now_Value = 0;}

    if(Motor_Close_Temp == 1)
    {
        Motor_Close_Temp = 0;

        valtage_filtered = 0;
        valtage_now_Value = 0;
    }
    switch (Dir)
    {
        case 0: Motor_Pwm = -valtage_now_Value; break;    //正方向
        case 1: Motor_Pwm =  valtage_now_Value; break;    //反方向
        default: break;
    }
//  printf("%d,%d,%d,%d\n\r",(int)valtage_adc_Value,(int)valtage_target_Value,(int)valtage_filtered,(int)Motor_Pwm);/*上位机看值,调参代码，勿删*/
    return Motor_Pwm;
}

/************************************************
 *  函数名称：自动控制
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年3月13日
 *  备    注：根据实际按键开关控制车与切换
 ************************************************/
void Auto_control(void)
{
    uint8_t Keybrake = gpio_get_level(KEY6);  // 杀车脚踏  Keybrake = 1 松开脚踏
    uint8_t switchover = gpio_get_level(KEY7); // 自动(科目)与手动(脚踏)切换  switchover = 1 为手动
    uint8_t back_car = gpio_get_level(KEY8);   // 倒车  back_car = 1 前进

    /*脚踏独立控制车关停*/
    if (Keybrake == 0) // 刹车优先级第一
    {
        ips200_show_string(180, 16*2, "Stop");
        Motor_Start = 0;        //关闭电机
        dispatch_car = 0;       //关闭发车标志位，
        Motor_Close_Temp = 1;   //清除由低通滤波产生的滞后影响
    }
    else
    {
        ips200_show_string(180, 16*2, "Go  ");
        Motor_Start = 1;        //开启电机
    }

    if (switchover == 1) // 不刹车与遥控开启情况由遥控选择模式 && Motor_Start == 1
    {
        Transform = 0;  //关闭科目 
        ips200_show_string(180, 16*3, "SD");
        if (uart_receiver.channel[2] == 192)
        {
            HotRC_control();
        }
        else
        {
            int16 Motor_Pwm = Manual_Control(back_car);
            Back_Motor_SetPwm(Motor_Pwm, Motor_Pwm);
        }
    }
    else if (switchover == 0) // 自动
    {
        ips200_show_string(180, 16*3, "KM");
        Transform = 1;  //开启科目
    }

    
}


/************************************************
 *  函数名称: 一阶低通滤波
 *  功能说明：
 *  参数说明：
 *  参   数：要滤波的原始值
 *  函数返回：滤波完成值
 *  修改时间：2026年3月16日
 *  备   注：
 ************************************************/
int32 Filter(int32 rawValue)
{
    static int32 filtered = 0;
    float alpha = 0.3f;

    filtered = (int32)(rawValue * alpha + filtered * (1 - alpha));
    return filtered;
}

/************************************************
 *  函数名称: 滤波器初始化
 *  功能说明：
 *  参数说明：
 *  参   数：结构体指针
 *  函数返回：无
 *  修改时间：2026年3月16日
 *  备   注：
 ************************************************/
void MeanFilter_Init(MeanFilter_TypeDef *filter)
{
    for (uint8 i = 0; i < FILTER_WINDOW_SIZE; i++)
    {
        filter->buffer[i] = 0;
    }
    filter->index = 0;
    filter->sum = 0;
    filter->is_full = 0;
}

/************************************************
 *  函数名称: 均值滤波器
 *  功能说明：
 *  参数说明：
 *  参   数：滤波器结构体指针
 *         要滤波的原始数据
 *  函数返回：滤波后的平均值
 *  修改时间：2026年3月16日
 *  备   注：
 ************************************************/
int32_t MeanFilter_Update(MeanFilter_TypeDef *filter, int32 new_data)
{
    int32 old_data = filter->buffer[filter->index];

    filter->sum = filter->sum - old_data + new_data;

    filter->buffer[filter->index] = new_data;

    filter->index++;
    if (filter->index >= FILTER_WINDOW_SIZE)
    {
        filter->index = 0;
        filter->is_full = 1;
    }

    uint8 count = (filter->is_full) ? FILTER_WINDOW_SIZE : (filter->index);

    return filter->sum / count;
}

/************************************************
 *  函数名称: karma滤波器
 *  功能说明： q: 过程噪声 (通常 0.001 ~ 0.01，越小越信任模型)
            r: 测量噪声 (通常 0.1 ~ 1.0，越大越不信任测量值)
 *  参数说明：
 *  参   数：滤波器结构体指针
 *         初始速度
 *  函数返回：无
 *  修改时间：2026年3月16日
 *  备   注：
 ************************************************/
void Kalman_Init(KalmanFilter *kf, float init_val, float q, float r)
{
    kf->x = init_val;
    kf->p = 1.0f;
    kf->q = q;
    kf->r = r;
    kf->k = 0.0f;
}
/************************************************
 *  函数名称: karma滤波器
 *  功能说明： q: 过程噪声 (通常 0.001 ~ 0.01，越小越信任模型)
            r: 测量噪声 (通常 0.1 ~ 1.0，越大越不信任测量值)
 *  参数说明：
 *  参   数：滤波器结构体指针
 *         编码器当前测得的速度 (含噪声)
 *  函数返回：无
 *  修改时间：2026年3月16日
 *  备   注：更新函数
 ************************************************/
float Kalman_Update(KalmanFilter *kf, float measurement)
{
    float x_pred = kf->x;
    float p_pred = kf->p + kf->q;

    kf->k = p_pred / (p_pred + kf->r);

    kf->x = x_pred + kf->k * (measurement - x_pred);

    kf->p = (1.0f - kf->k) * p_pred;

    return kf->x;
}

MeanFilter_TypeDef adc_filter;
KalmanFilter Kalman_Filter;


