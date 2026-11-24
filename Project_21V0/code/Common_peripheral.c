/*
 * Common_peripheral.c
 *
 *  Created on: 2025年6月12日
 *      Author: 吉祥止止
 */


#include "zf_common_headfile.h"

//==============================================LED和按键变量定义区===================================================
uint8 key1_state = 1;                                // 按键动作状态
uint8 key2_state = 1;                                // 按键动作状态
uint8 key3_state = 1;                                // 按键动作状态
uint8 key4_state = 1;                                // 按键动作状态
uint8 key5_state = 1;                                // 按键动作状态

uint8 sw1_state = 1;                                 // 拨码开关动作状态
uint8 sw2_state = 1;                                 // 拨码开关动作状态

uint8 key1_state_last = 0;                           // 上一次按键动作状态
uint8 key2_state_last = 0;                           // 上一次按键动作状态
uint8 key3_state_last = 0;                           // 上一次按键动作状态
uint8 key4_state_last = 0;                           // 上一次按键动作状态
uint8 key5_state_last = 0;                           // 上一次按键动作状态

uint8 sw1_state_last = 0;                            // 上一次拨码开关动作状态
uint8 sw2_state_last = 0;                            // 上一次拨码开关动作状态

//按键开关标志
uint8 key1_flag;
uint8 key2_flag;
uint8 key3_flag;
uint8 key4_flag;
uint8 key5_flag;

//拨码开关标志
int sw1_flag=0;
int sw2_flag=0;


//===================================================蜂鸣器===================================================

uint16 Buzzer_Count;    // 蜂鸣器响时间计数
uint16 Buzzer_ScanNum;  // 定时器扫描计数
uint8_t Buzzer_Start;   // 蜂鸣器是否开始响
uint8 Buzzer_ScanTemp;  //

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

// 定时器扫描函数
//定时器扫描(定时器设置每1ms调用一次)
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

    sw1_state_last = sw1_state;
    sw2_state_last = sw2_state;

    //读取当前按键状态
    key1_state = gpio_get_level(KEY1);
    key2_state = gpio_get_level(KEY2);
    key3_state = gpio_get_level(KEY3);
    key4_state = gpio_get_level(KEY4);
    key5_state = gpio_get_level(KEY5);

    sw1_state = gpio_get_level(sw1);
    sw2_state = gpio_get_level(sw2);

    //检测到按键按下之后  并放开置位标志位
    if(key1_state && !key1_state_last)   {key1_flag = 1;}//按键按下完成对应标志位会置1
    if(key2_state && !key2_state_last)   {key2_flag = 1;}
    if(key3_state && !key3_state_last)   {key3_flag = 1;}
    if(key4_state && !key4_state_last)   {key4_flag = 1;}
    if(key5_state && !key5_state_last)   {key5_flag = 1;}

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
    gpio_toggle_level(LED2);
    gpio_toggle_level(LED3);
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
//    HotRC_control_Init();//遥控初始化

    ips200_set_color(RGB565_WHITE, RGB565_BLACK);   //设置彩屏背景颜色
    ips200_init(IPS200_TYPE);   //彩屏初始化
    imu660ra_init();            //陀螺仪初始化
    quaternion_init();          //四元素解算初始化
    GPS_init();                 //GPS初始化

    GPS_PID_Init(16.8,0.68);    //GPS转向位置环PID初始化12.8,0.68

    adc_init(VOLTAGE_PORT, ADC_12BIT);  //ADC初始化

    All_MOtor_init();                   //电机初始化
    Encoder_Init(TIM2_ENCODER);         //编码器初始化
    Encoder_Init(TIM4_ENCODER);         //编码器初始化
    Encoder_Init(TIM5_ENCODER);         //编码器初始化

    All_PID_Init();
    MeanFilter_Init(&adc_filter);
    Kalman_Init(&Kalman_Filter,0,0.005,0.5);

    KM3_All_PID_Init();

    //科二
//    Voice_Init();                   //初始化麦克风、定时器
//    WIFI_UART_Init();               //WIFI初始化
//    dot_matrix_screen_init();       //点阵屏幕初始化
//
//    ips200_show_rgb565_image(0, 16*2, (const uint16*)DORO_SLEEP, 240, 320, 240, 288, 0);//播放doro睡觉，表示没有想服务端发送消息
//    ips200_show_string(0, 16*0,"Latest_Comand:");   //普通指令
//    ips200_show_string(0, 16*1,"New_CmdArr:");      //普通指令
//    ips200_show_uint(8*15, 16*0,nomal_command,2);
//    for(uint8 i=0;i<5;i++)
//    {
//        ips200_show_uint(8*(12+i*3), 16*1,Command_buffer[i],2);
//    }

}

void KM2_Init(void)
{
//    //科二
//    Voice_Init();                   //初始化麦克风、定时器
//    WIFI_UART_Init();               //WIFI初始化
//    dot_matrix_screen_init();       //点阵屏幕初始化
//
//    ips200_show_rgb565_image(0, 16*2, (const uint16*)DORO_SLEEP, 240, 320, 240, 288, 0);//播放doro睡觉，表示没有想服务端发送消息
//    ips200_show_string(0, 16*0,"Latest_Comand:");   //普通指令
//    ips200_show_string(0, 16*1,"New_CmdArr:");      //普通指令
//    ips200_show_uint(8*15, 16*0,nomal_command,2);
//    for(uint8 i=0;i<5;i++)
//    {
//        ips200_show_uint(8*(12+i*3), 16*1,Command_buffer[i],2);
//    }
}

