/*
 * Common_peripheral.c
 *
 *  Created on: 2025年6月12日
 *      Author: LiangZhaoqi
 */


#include "zf_common_headfile.h"

//==============================================LED和按键变量定义区===================================================
uint8 key1_state = 1;                                // 按键动作状态
uint8 key2_state = 1;                                // 按键动作状态
uint8 key3_state = 1;                                // 按键动作状态
uint8 key4_state = 1;                                // 按键动作状态
uint8 key5_state = 1;                                // 按键动作状态

uint8 key9_state = 1;                                // 按键动作状态

uint8 sw1_state = 1;                                 // 拨码开关动作状态
uint8 sw2_state = 1;                                 // 拨码开关动作状态

uint8 key1_state_last = 0;                           // 上一次按键动作状态
uint8 key2_state_last = 0;                           // 上一次按键动作状态
uint8 key3_state_last = 0;                           // 上一次按键动作状态
uint8 key4_state_last = 0;                           // 上一次按键动作状态
uint8 key5_state_last = 0;                           // 上一次按键动作状态

uint8 key9_state_last = 0;                           // 上一次按键动作状态

uint8 sw1_state_last = 0;                            // 上一次拨码开关动作状态
uint8 sw2_state_last = 0;                            // 上一次拨码开关动作状态

//按键开关标志
uint8 key1_flag;
uint8 key2_flag;
uint8 key3_flag;
uint8 key4_flag;
uint8 key5_flag;
uint8 key9_flag;

//拨码开关标志
uint8  sw1_flag=0;
uint8  sw2_flag=0;


//===================================================蜂鸣器===================================================

static uint16 Buzzer_Count;    // 蜂鸣器响时间计数
static uint16 Buzzer_ScanNum;  // 定时器扫描计数
static uint8_t Buzzer_Start;   // 蜂鸣器是否开始响
static uint8 Buzzer_ScanTemp;  //

/************************************************
 *  函数名称：蜂鸣器初始化：Buzzer_init(void)
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月12日
 *  备    注：
 ************************************************/
void Buzzer_init(void)//蜂鸣器初始化
{
    gpio_init(BUZZER_PIN, GPO, 0, GPO_PUSH_PULL);
}

/************************************************
 *  函数名称: 蜂鸣器开：Buzzer_ON(void)
 *  功能说明：引脚：BUZZER_PIN  (P33_10)
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月25日
 *  备   注：
 ************************************************/
void Buzzer_ON(void)
{
    gpio_set_level(BUZZER_PIN,1);
}

/************************************************
 *  函数名称: 蜂鸣器关：Buzzer_OFF(void)
 *  功能说明：引脚：BUZZER_PIN  (P33_10)
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月25日
 *  备   注：
 ************************************************/
void Buzzer_OFF(void)
{
    gpio_set_level(BUZZER_PIN,0);
}

/************************************************
 *  函数名称: 蜂鸣器设置响时间：Buzzer_SetTimer(uint8_t Start,uint32 Count)
 *  功能说明：引脚：BUZZER_PIN  (P33_10)
 *  参数说明：
 *  参   数：Count：蜂鸣器响时间
 *  函数返回：无
 *  修改时间：2025年6月25日
 *  备   注：
 ************************************************/
void Buzzer_SetTimer(uint16 Count)
{
    Buzzer_Count = Count;   //给中断变量赋值
    Buzzer_Start = 1;       //打开蜂鸣器
    Buzzer_ScanTemp=1;
}

/************************************************
 *  函数名称: 定时器扫描蜂鸣器函数
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月25日
 *  备   注：定时器设置每1ms调用一次
 ************************************************/
void Buzzer_scan(void)
{
    if(Buzzer_Start == 1)
    {
        Buzzer_ScanNum++;
        Buzzer_ON(); // 蜂鸣器响
        if((Buzzer_ScanNum >= Buzzer_Count) && Buzzer_ScanTemp ==1)
        {
            Buzzer_ScanNum = 0;
            Buzzer_OFF(); // 蜂鸣器停止响
            Buzzer_Start = 0;
            Buzzer_ScanTemp = 0;
        }
    }
}

/************************************************
 *  函数名称: 蜂鸣器的自检函数：Buzzer_check(int time)
 *  功能说明：引脚：BUZZER_PIN  (P33_10)
 *  参数说明：范围：0到2147483647
 *  参   数：time2自检时间,单位：ms
 *  函数返回：无
 *  修改时间：2025年6月12日
 *  备   注：用定时器作为蜂鸣器响应时间
 ************************************************/
void Buzzer_check(uint16 time)//蜂鸣器的自检函数
{
    Buzzer_SetTimer(time);
}

//==============================================按键===================================================

/************************************************
 *  函数名称: 按键与拨码开关初始化：Key_init(void)
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月12日
 *  备   注：
 ************************************************/
void Key_init(void)//按键与拨码开关初始化
{
   gpio_init(KEY1, GPI, GPIO_HIGH, GPI_PULL_UP);           // 初始化 KEY1 输入 默认高电平 上拉输入
   gpio_init(KEY2, GPI, GPIO_HIGH, GPI_PULL_UP);           // 初始化 KEY2 输入 默认高电平 上拉输入
   gpio_init(KEY3, GPI, GPIO_HIGH, GPI_PULL_UP);           // 初始化 KEY3 输入 默认高电平 上拉输入
   gpio_init(KEY4, GPI, GPIO_HIGH, GPI_PULL_UP);           // 初始化 KEY4 输入 默认高电平 上拉输入
   gpio_init(KEY5, GPI, GPIO_HIGH, GPI_PULL_UP);           // 初始化 KEY5 输入 默认高电平 上拉输入

   gpio_init(KEY6, GPI, GPIO_HIGH, GPI_PULL_UP);           // 初始化 KEY6 输入 默认高电平 上拉输入
   gpio_init(KEY7, GPI, GPIO_HIGH, GPI_PULL_UP);           // 初始化 KEY7 输入 默认高电平 上拉输入
   gpio_init(KEY8, GPI, GPIO_HIGH, GPI_PULL_UP);           // 初始化 KEY8 输入 默认高电平 上拉输入
   gpio_init(KEY9, GPI, GPIO_HIGH, GPI_PULL_UP);           // 初始化 KEY8 输入 默认高电平 上拉输入

   gpio_init(sw1, GPI, GPIO_HIGH, GPI_PULL_UP);            //上拉输入，高电平    拨码开关
   gpio_init(sw2, GPI, GPIO_HIGH, GPI_PULL_UP);            //上拉输入，高电平
}

/************************************************
 *  函数名称：按键扫描：key_scan(void)
 *  功能说明：保证调用时间不小于10ms
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月12日
 *  备   注：标志位置位之后，可以使用标志位执行自己想要做的事件
 ************************************************/
void key_Scan(void)//按键扫描
{
    //使用此方法优点在于，不需要使用while(1) 等待，避免处理器资源浪费
    //保存按键状态
    key1_state_last = key1_state;
    key2_state_last = key2_state;
    key3_state_last = key3_state;
    key4_state_last = key4_state;
    key5_state_last = key5_state;

    key9_state_last = key9_state;

    sw1_state_last = sw1_state;
    sw2_state_last = sw2_state;

    //读取当前按键状态
    key1_state = gpio_get_level(KEY1);
    key2_state = gpio_get_level(KEY2);
    key3_state = gpio_get_level(KEY3);
    key4_state = gpio_get_level(KEY4);
    key5_state = gpio_get_level(KEY5);

    key9_state = gpio_get_level(KEY9);

    sw1_state = gpio_get_level(sw1);
    sw2_state = gpio_get_level(sw2);

    //检测到按键按下之后  并放开置位标志位
    if(key1_state && !key1_state_last)   {key1_flag = 1;}//按键按下完成对应标志位会置1
    if(key2_state && !key2_state_last)   {key2_flag = 1;}
    if(key3_state && !key3_state_last)   {key3_flag = 1;}
    if(key4_state && !key4_state_last)   {key4_flag = 1;}
    if(key5_state && !key5_state_last)   {key5_flag = 1;}

    if(key9_state && !key9_state_last)   {key9_flag = 1;}

    if(sw1_state && !sw1_state_last)   {sw1_flag = 1;}
    if(sw2_state && !sw2_state_last)   {sw2_flag = 1;}
}

//==============================================LED===================================================
/************************************************
 *  函数名称: LED初始化：LED_init(void)
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月12日
 *  备   注：
 ************************************************/
void LED_init(void)//LED初始化
{
//    gpio_init(LED1, GPO, GPIO_HIGH, GPO_PUSH_PULL);         // 初始化 LED1 输出 默认高电平 推挽输出模式
    gpio_init(LED2, GPO, GPIO_LOW, GPO_PUSH_PULL);         // 初始化 LED2 输出 默认高电平 推挽输出模式
    gpio_init(LED3, GPO, GPIO_HIGH, GPO_PUSH_PULL);         // 初始化 LED3 输出 默认高电平 推挽输出模式
    gpio_init(LED4, GPO, GPIO_LOW, GPO_PUSH_PULL);         // 初始化 LED4 输出 默认高电平 推挽输出模式
}

/************************************************
 *  函数名称: LED闪烁：LED_Tick(void)
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月12日
 *  备   注：
 ************************************************/
void LED_Tick(void)
{
//    gpio_toggle_level(LED1);
//    gpio_toggle_level(LED2);
//    gpio_toggle_level(LED3);
    gpio_toggle_level(LED4);
}


//==============================================Delay===================================================
uint16 delay_Count;    // 延迟时间计数
uint16 delay_ScanNum;  // 定时器扫描计数
uint8_t delay_Start;   // 延迟开启
uint8 delay_ScanTemp;  // 定时器到达时间置1

/************************************************
 *  函数名称: 定时器计时：void Timer_delay_scan(void)
 *  功能说明：通过全局变量置delay_Start ‘1’来进行无堵塞式延迟
 *  参数说明：
 *  参   数：time：要延迟的时间，单位ms
 *  函数返回：时间到时返回1
 *  修改时间：2026年3月8日
 *  备   注：定时器调用
 ************************************************/
uint8 Timer_delay_ms(uint16 time)
{
    delay_Count = time;
    return delay_ScanTemp;
}

/************************************************
 *  函数名称: 定时器计时：void Timer_delay_scan(void)
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年3月8日
 *  备   注：定时器调用
 ************************************************/
void Timer_delay_scan(void)
{
    if(delay_Start == 1)
    {
        delay_ScanNum++;

        if((delay_ScanNum >= delay_Count))
        {
            delay_ScanNum = 0;
            delay_Start = 0;
            delay_ScanTemp = 1;
        }
        else
        {
            delay_ScanTemp = 0;
        }
    }
}

/************************************************
 *  函数名称：主循环频率测量：main_loop_freq_update(void)
 *  功能说明：主循环每执行一次调用一次,返回当前主循环频率(Hz)
 *  参数说明：
 *  参   数：无
 *  函数返回：当前主循环频率(Hz)
 *  修改时间：2026年8月15日
 *  备    注：累计 1 秒内的循环次数,满 1 秒时按 次数/秒数 计算并清零重计
 ************************************************/
float main_loop_freq_update(void)
{
    static uint32 last_us = 0;      // 上次结算时刻(微秒)
    static uint16 loop_cnt = 0;     // 本统计段内主循环执行次数
    static float  loop_freq = 0.0f; // 当前主循环频率

    uint32 now_us = system_getval_us(); // 当前微秒时刻
    loop_cnt++;                         // 本次循环计入统计

    if (last_us == 0) {
        // 首次调用:仅记录起始时刻
        last_us = now_us;
        loop_cnt = 0;
        return loop_freq;
    }

    // 距上次结算经过的微秒数(含 32 位计数器溢出回绕保护)
    uint32 elapsed = (now_us >= last_us)
        ? (now_us - last_us)
        : (0xFFFFFFFF - last_us + now_us + 1);

    // 满 1 秒结算一次:频率 = 循环次数 / 秒数
    if (elapsed >= 1000000u) {
        loop_freq = (float)loop_cnt * 1000000.0f / (float)elapsed;
        loop_cnt = 0;
        last_us = now_us;
    }
    return loop_freq;
}

/************************************************
 *  函数名称: 总初始化：ALL_init(void)
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年3月6日
 *  备   注：
 ************************************************/
void ALL_init(void)//总初始化
{
    //外设初始化代码等
    Buzzer_init();  //蜂鸣器初始化
    LED_init();     //LED初始化
    Key_init();     //按键初始化
    HotRC_control_Init();//遥控初始化

    ips200_set_color(RGB565_WHITE, RGB565_BLACK);   //设置彩屏背景颜色
    ips200_init(IPS200_TYPE);           //彩屏初始化

    IMU_init();                         //IMU初始化
    GPS_init();                         //GPS初始化
    All_MOtor_init();                   //电机初始化
    Angle_Encoder_Init();               //前转向电机角度编码器初始化
    Encoder_Init(TIM2_ENCODER);         //右编码器初始化
    Encoder_Init(TIM5_ENCODER);         //左编码器初始化
    adc_init(VOLTAGE_PORT, ADC_12BIT);  //ADC初始化

    CI1302_init();                      //CI1302 语音(UART19)
    by9301_init();                      //BY9301 鸣笛(UART10)
    // dot_matrix_screen_init();           //点阵屏幕初始化

    GPS_PID_Init(16.8,0.68);            //GPS转向位置环PID初始化12.8,0.68


    All_PID_Init();
    MeanFilter_Init(&adc_filter);
    Kalman_Init(&Kalman_Filter,0,0.005,0.5);

    menu_key_init(MENU_SCAN_PERIOD_MS);  // 【新增】旋钮按键(P11_11)初始化，供短按/长按扫描

}

