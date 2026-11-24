/*
 * Algorithmic_Filtering.c
 *
 *  Created on: 2026年3月24日
 *      Author: liang2281
 */

#include "Algorithmic_Filtering.h"

extern uint8 dispatch_car; //发车标志位(main定义，发车按键按下，1秒后发车，科目完成后标志位清0)

uint8_t Key6_Num,Key7_Num,Key8_Num;
uint8 Transform = 0;    //科目与手动转换标志位
uint8 Motor_Start = 0;  //电机开启标志位
uint8 Motor_Close_Temp = 0;//清除由低通滤波产生的滞后影响标志位

/************************************************
 *  函数名称: 值线性映射：uint16_t map_voltage_adc(uint16_t adc_val)
 *  功能说明：将 ADC 值线性映射到 OUT_MIN - OUT_MAX
 *  参数说明：adc_val 原始 ADC 值 (IN_MIN - IN_MAX)
 *  参   数：原始 ADC 值
 *  函数返回：将 ADC 值线性映射值
 *  修改时间：2026年3月8日
 *  备   注：这里输入与输出范围已经根据当前车的脚踏进行限幅与范围调整
 ************************************************/
uint16_t map_voltage_adc(uint16_t adc_val)
{
    // 定义常量
    const int16_t IN_MIN  = 2040;
    const int16_t IN_MAX  = 4030;
    const int16_t OUT_MIN = 0;
    const int16_t OUT_MAX = 1000;

    // 输入限幅 (Clamping)：防止负数或超出最大值导致计算错误
    if (adc_val <= IN_MIN) return OUT_MIN;
    if (adc_val >= IN_MAX) return OUT_MAX;

    // 核心计算 (使用 32位整数防止溢出)
    // 公式: (x - in_min) * (out_max - out_min) / (in_max - in_min)
    // 分子部分：(adc_val - 1700) * 1000
    // 最大值约为 1800 * 1000 = 1,800,000，这超过了 16位整数的范围 (32767)
    // 所以必须强制转换为 int32_t 进行乘法
    int32_t numerator = ((int32_t)adc_val - IN_MIN) * (OUT_MAX - OUT_MIN);
    int32_t denominator = (IN_MAX - IN_MIN); // 1800

    // 实现四舍五入 (Round Half Up)
    // 技巧：在除法之前加上分母的一半 (denominator / 2)
    // 1800 / 2 = 900
    int32_t result = (numerator + (denominator / 2)) / denominator;

    // 返回结果 (结果肯定在 0-1000 之间，用 uint16_t 足够)
    return (uint16_t)result;
}

/************************************************
 *  函数名称：脚踏控制车运动：void Manual_Control(void)
 *  功能说明：采集脚踏传感器ADC值与滤波，并进行一阶低通滤波
 *  参数说明：调节参数 α 打开注释 printf 上位机看值
 *  参   数：Dir是电机方向选择，0：正方向，1：反方向
 *  函数返回：无
 *  修改时间：2026年3月13日
 *  备    注：根据实际情况修改参数，目前加速反应快，减速反应慢，避免减速反应过快像刹车
 ************************************************/
int16 Manual_Control(uint8 Dir)
{
    uint16 valtage_adc_Value = 0;           //原始ADC均值滤波的值
    uint16 valtage_target_Value = 0;        // 转换的目标值
    static uint16_t valtage_filtered = 0;   // 保存上一次滤波结果（初始为0）
    uint16 valtage_now_Value = 0;           // 最终输出值
    float current_alpha = 0.0f;             // 滤波系数（0<α<1，α越小，平滑效果越强）
    int16 Motor_Pwm = 0;                    //返回的电机速度
    // 定义两个不同的系数
    const float alpha_fast = 0.2f;          // 加速系数：响应快
    const float alpha_slow = 0.025f;        // 减速系数：惯性大

    valtage_adc_Value = adc_mean_filter_convert(VOLTAGE_PORT,5);  //获取ADC值并滤波

    valtage_target_Value = map_voltage_adc(valtage_adc_Value);    //ADC 值线性映射

    if(valtage_target_Value < 50){valtage_target_Value = 0;}      //设置最小启动电机电压

    //判断是加速还是减速，动态选择系数
    if (valtage_target_Value >= valtage_filtered) // 目标值 >= 当前值 -> 加速过程 (或保持) -> 用大系数，快速响应
    {
        current_alpha = alpha_fast;
    }
    else    // 目标值 < 当前值 -> 减速过程 (松开脚踏) -> 用小系数，缓慢下降
    {
        current_alpha = alpha_slow;
    }

    /*一阶低通滤波（加权平均，平滑效果会更明显）公式：本次滤波值 = 原始值×α + 上次滤波值×(1-α)*/
    valtage_now_Value = (uint16_t)(valtage_target_Value * current_alpha + valtage_filtered * (1.0f - current_alpha));
    valtage_filtered = valtage_now_Value;

    // 如果目标已经是0，且当前滤波值已经非常小（比如小于2），直接强制设为0
    // 防止因为浮点精度或极小的尾数导致电机无法完全停止
    if (valtage_target_Value == 0 && valtage_now_Value < 2) {valtage_now_Value = 0;}

    if(Motor_Close_Temp == 1)   //清除踩刹车时由低通滤波产生的滞后影响
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

//    printf("%d,%d,%d,%d\n\r",(int)valtage_adc_Value,(int)valtage_target_Value,(int)valtage_filtered,(int)Motor_Pwm);/*上位机看值,调参代码，勿删*/
    return Motor_Pwm;
}

void Auto_control(void)
{
    int16 Motor_Pwm;
    Key6_Num = gpio_get_level(KEY6);    //杀车脚踏  Key6_Num = 1 松开脚踏
    Key7_Num = gpio_get_level(KEY7);    //自动(科目)与手动(脚踏)切换  Key7_Num = 1 为手动
    Key8_Num = gpio_get_level(KEY8);    //倒车  Key8_Num = 1 前进

    /*遥控与脚踏独立控制车关停*/
    if(Key6_Num == 0)//刹车优先级第一
    {
        ips200_show_string(120, 16*2, "Stop");
        Motor_Start = 0;        //关闭电机
        dispatch_car = 0;       //关闭发车标志位，
        Motor_Close_Temp = 1;   //清除由低通滤波产生的滞后影响
    }
    else
    {
        ips200_show_string(120, 16*2, "Go  ");
        Motor_Start = 1;        //开启电机
    }

    if(Key7_Num == 1 && Motor_Start == 1)//不刹车与遥控开启情况由遥控选择模式
    {
        Transform = 0;  //关闭科目 //Key7_Num = 1 为手动(脚踏)
        ips200_show_string(120, 16*3, "SD");
        Motor_Pwm = Manual_Control(Key8_Num);
        Back_Motor_SetPwm(Motor_Pwm,Motor_Pwm);
    }
    else if(Key7_Num == 0)  //Key7_Num = 0 自动
    {
        ips200_show_string(120, 16*3, "KM");
        Transform = 1;  //开启科目
    }
    else if(Motor_Start == 0)
    {
        Back_Motor_SetPwm(0,0);
    }

}


/************************************************
 *  函数名称: 一阶低通滤波：uint16 Filter(uint16 rawValue)
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
 *  函数名称: 初始化滤波器：void MeanFilter_Init(MeanFilter_TypeDef *filter)
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
 *  函数名称: 均值滤波器：int32_t MeanFilter_Update(MeanFilter_TypeDef *filter, int32_t new_data)
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
    //减去即将被覆盖的旧数据
    int32 old_data = filter->buffer[filter->index];

    //更新总和：总和 = 总和 - 旧数据 + 新数据
    filter->sum = filter->sum - old_data + new_data;

    //将新数据存入缓冲区
    filter->buffer[filter->index] = new_data;

    //移动索引(环形队列)
    filter->index++;
    if (filter->index >= FILTER_WINDOW_SIZE)
    {
        filter->index = 0;
        filter->is_full = 1; // 标记已填满一轮
    }

    //计算平均值
    //如果还没填满一圈，除以实际填入的个数，否则除以窗口大小
    uint8 count = (filter->is_full) ? FILTER_WINDOW_SIZE : (filter->index);

    //整数除法，如果需要小数，可以返回 sum * 10 / count 然后由上层处理小数位
    return filter->sum / count;
}

// 初始化
// init_val: 初始速度
// q: 过程噪声 (通常 0.001 ~ 0.01，越小越信任模型)
// r: 测量噪声 (通常 0.1 ~ 1.0，越大越不信任测量值)
void Kalman_Init(KalmanFilter *kf, float init_val, float q, float r)
{
    kf->x = init_val;
    kf->p = 1.0f;
    kf->q = q;
    kf->r = r;
    kf->k = 0.0f;
}

// 更新函数
// measurement: 编码器当前测得的速度 (含噪声)
float Kalman_Update(KalmanFilter *kf, float measurement)
{
    // --- 1. 预测阶段 (Predict) ---
    // 假设模型是恒速模型: x_pred = x_prev (也可以加入加速度项)
    float x_pred = kf->x;
    float p_pred = kf->p + kf->q; // 预测的不确定性增加

    // --- 2. 更新阶段 (Update) ---
    // 计算卡尔曼增益 K = P_pred / (P_pred + R)
    // K 的范围在 0~1 之间。测量噪声 R 越大，K 越小（越不信测量值）
    kf->k = p_pred / (p_pred + kf->r);

    // 更新状态估计: x = x_pred + K * (measurement - x_pred)
    kf->x = x_pred + kf->k * (measurement - x_pred);

    // 更新协方差: P = (1 - K) * P_pred
    kf->p = (1.0f - kf->k) * p_pred;

    return kf->x;
}

MeanFilter_TypeDef adc_filter;
KalmanFilter Kalman_Filter;


