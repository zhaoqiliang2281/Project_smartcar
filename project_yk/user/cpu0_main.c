/*********************************************************************************************************************
* TC264 Opensourec Library 即（TC264 开源库）是一个基于官方 SDK 接口的第三方开源库
* Copyright (c) 2022 SEEKFREE 逐飞科技
*
* 本文件是 TC264 开源库的一部分
*
* TC264 开源库 是免费软件
* 您可以根据自由软件基金会发布的 GPL（GNU General Public License，即 GNU通用公共许可证）的条款
* 即 GPL 的第3版（即 GPL3.0）或（您选择的）任何后来的版本，重新发布和/或修改它
*
* 本开源库的发布是希望它能发挥作用，但并未对其作任何的保证
* 甚至没有隐含的适销性或适合特定用途的保证
* 更多细节请参见 GPL
*
* 您应该在收到本开源库的同时收到一份 GPL 的副本
* 如果没有，请参阅<https://www.gnu.org/licenses/>
*
* 额外注明：
* 本开源库使用 GPL3.0 开源许可证协议 以上许可申明为译文版本
* 许可申明英文版在 libraries/doc 文件夹下的 GPL3_permission_statement.txt 文件中
* 许可证副本在 libraries 文件夹下 即该文件夹下的 LICENSE 文件
* 欢迎各位使用并传播本程序 但修改内容时必须保留逐飞科技的版权声明（即本声明）
*
* 文件名称          cpu0_main
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          ADS v1.9.4
* 适用平台          TC264D
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2022-09-15       pudding            first version
********************************************************************************************************************/
#include "zf_common_headfile.h"
#pragma section all "cpu0_dsram"
// 将本语句与#pragma section all restore语句之间的全局变量都放在CPU0的RAM中

#define LORA3A22_HotRC   (1)        //1是：LORA3A22  0是：HotRC


// **************************** 代码区域 ****************************

uint8 flag_control = 0;
int8 flag_go = 1;   //科目标志位
uint8 dispatch_car = 0; //发车标志位


uint8 while_flag = 1;

#define VOLTAGE_PORT    ADC0_CH11_A11 // 定义检测端口

uint16 valtage_adc = 0;
float valtage_now = 0;
float valtage_nowV = 0;
float valtage_Value = 0;

/************************************************
 *  函数名称: 一阶低通滤波
 *  功能说明：
 *  参数说明：
 *  参   数：要滤波的原始值
 *  函数返回：滤波完成值
 *  修改时间：2026年3月16日
 *  备   注：
 ************************************************/
float Filter(float rawValue)
{
    static float filtered = 0;
    float alpha = 0.2f;

    filtered = (float)(rawValue * alpha + filtered * (1 - alpha));
    return filtered;
}


void core0_main(void)
{
    clock_init();                   // 获取时钟频率<务必保留>
    debug_init();                   // 初始化默认调试串口
    // 此处编写用户代码 例如外设初始化代码等        ips200_draw_line(188, 120, 0, 120, RGB565_WHITE);

    ALL_init();
    system_delay_ms(10);
    ips200_show_rgb565_image(0, 120, (const uint16 *)gImage_seekfree_logo, 240, 80, 240, 80, 0);
    system_delay_ms(10);
    ips200_show_chinese(3 * 8, 16 * 1, 32, (const uint8 *)Electric_Innovation32, 6, RGB565_GREEN); // 显示汉字
    system_delay_ms(10);
    ips200_show_rgb565_image(0, 80, (const uint16 *)gImage_c, 240, 240, 240, 240, 0);
    system_delay_ms(10);


    adc_init(VOLTAGE_PORT, ADC_12BIT);                                          // 建议电磁传感器用 ADC1 电池检测用 ADC2

    // 此处编写用户代码 例如外设初始化代码等
    cpu_wait_event_ready();         // 等待所有核心初始化完毕
    while (TRUE)
    {
        valtage_adc = adc_convert(VOLTAGE_PORT);
        valtage_now = 36.3 * valtage_adc / 4096;

        valtage_Value = Filter(valtage_now);

        ips200_show_float(0, 16 * 0, valtage_Value, 2,3);

        // 此处编写需要循环执行的代码
        while(while_flag)   //保护锁  开锁前也能看到电池电压
        {
            static uint8 sum=0,a=0,b=0;

            valtage_adc = adc_convert(VOLTAGE_PORT);
            valtage_now = 36.3 * valtage_adc / 4096;

            valtage_Value = Filter(valtage_now);
            ips200_show_float(0, 16 * 0, valtage_Value, 2,3);

            if(key1_flag == 1)
            {
                key1_flag=0;
                Buzzer_check(50);
                a++;
            }
            if(key2_flag == 1)
            {
                b+=2;
                key2_flag=0;
                Buzzer_check(50);
            }
            sum = a+b;
//            ips200_show_int(0, 16 * 0, sum, 5);
            if(sum == 3)    //按下第一个按键三次解锁
            {
                ips200_clear();
                Buzzer_check(50);
                while_flag = 0;
                break;
            }
        }

#if LORA3A22_HotRC
        lora3a22_process_fifo_data();//
        lora3a22_data_show();//
        lora3a22_control(); //遥控控制车
//      vofa_firewater_send();//

#else

        /*遥控接收数据显示*/
        HotRC_receive_data_Show();//
        HotRC_control();//

#endif

//        system_delay_ms(10);

        // 此处编写需要循环执行的代码
    }
}

#pragma section all restore
// **************************** 代码区域 ****************************

// **************************** PIT中断函数 ****************************
IFX_INTERRUPT(cc60_pit_ch0_isr, 0, CCU6_0_CH0_ISR_PRIORITY)     //中断时间为10ms
{
    interrupt_global_enable(0);                     // 开启中断嵌套
    pit_clear_flag(CCU60_CH0);

    //编码器采集
    encoder[0] = encoder_get_count(ENCODER1_TIM);       // 采集对应编码器数据
    encoder_clear_count(ENCODER1_TIM);                  // 清除对应计数

    //按键扫描
    key_scan();
}


IFX_INTERRUPT(cc60_pit_ch1_isr, 0, CCU6_0_CH1_ISR_PRIORITY) //1ms
{
    interrupt_global_enable(0);                     // 开启中断嵌套
    pit_clear_flag(CCU60_CH1);

    Buzzer_scan();              //蜂鸣器扫描
    Timer_delay_scan();         //延迟delay扫描
}

IFX_INTERRUPT(cc61_pit_ch0_isr, 0, CCU6_1_CH0_ISR_PRIORITY)
{
    interrupt_global_enable(0);                     // 开启中断嵌套
    pit_clear_flag(CCU61_CH0);


}
