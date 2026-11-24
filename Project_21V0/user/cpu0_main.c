/*********************************************************************************************************************
* TC387 Opensourec Library 即（TC387 开源库）是一个基于官方 SDK 接口的第三方开源库
* Copyright (c) 2022 SEEKFREE 逐飞科技
*
* 本文件是 TC387 开源库的一部分
*
* TC387 开源库 是免费软件
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
* 适用平台          TC387QP
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2022-11-04       pudding            first version
********************************************************************************************************************/
#include "zf_common_headfile.h"
#pragma section all "cpu0_dsram"
// 将本语句与#pragma section all restore语句之间的全局变量都放在CPU0的RAM中

// *************************** 例程硬件连接说明 ***************************
//      模块管脚            单片机管脚
//      BL                  查看 zf_device_ips200_parallel8.h 中 IPS200_BL_PIN 宏定义  默认 P15_3
//      CS                  查看 zf_device_ips200_parallel8.h 中 IPS200_CS_PIN 宏定义  默认 P15_5
//      RST                 查看 zf_device_ips200_parallel8.h 中 IPS200_RST_PIN 宏定义 默认 P15_1
//      RS                  查看 zf_device_ips200_parallel8.h 中 IPS200_RS_PIN 宏定义  默认 P15_0
//      WR                  查看 zf_device_ips200_parallel8.h 中 IPS200_WR_PIN 宏定义  默认 P15_2
//      RD                  查看 zf_device_ips200_parallel8.h 中 IPS200_RD_PIN 宏定义  默认 P15_4
//      D0-D7               查看 zf_device_ips200_parallel8.h 中 IPS200_Dx_PIN 宏定义  默认 P11_9/P11_10/P11_11/P11_12/P13_0/P13_1/P13_2/P13_3
//      GND                 核心板电源地 GND
//      3V3                 核心板 3V3 电源



// *************************** 例程测试说明 ***************************
// 1.核心板烧录本例程 插在主板上 2寸IPS 显示模块插在主板的屏幕接口排座上 请注意引脚对应 不要插错
// 2.电池供电 上电后 2寸IPS 屏幕亮起 显示字符数字浮点数和波形图
// 3.判断屏幕为SPI屏幕或者并口屏幕: 查看屏幕背面的PCB丝印，如果带有SPI字样就为SPI屏幕，否则为并口屏幕
//   例程文件夹下有不同类型2寸屏幕的区别示意图 "SPI和并口2寸屏幕区别.png"
//   修改下方的IPS200_TYPE宏定义来适配对应屏幕的驱动
// 如果发现现象与说明严重不符 请参照本文件最下方 例程常见问题说明 进行排查

// **************************** 代码区域 ****************************

uint8 dispatch_car = 0; //发车标志位

int Num;

uint8 Km13 = 0;

uint8 one_flag = 0;

int32 Read_Encodernum0,Read_Encodernum1,Read_Encodernum2,Read_Encodernum3;

int32 RxPacket_Data[4] = {0};

int core0_main(void)
{
    clock_init();                   // 获取时钟频率<务必保留>
    debug_init();                   // 初始化默认调试串口
    // 此处编写用户代码 例如外设初始化代码等

    ALL_init();

    NRF24L01_Init();
    //定时器中断开启
    pit_ms_init(CCU60_CH0, 10);     //开启60中断0初始化
    pit_ms_init(CCU60_CH1, 1);      //开启60中断1初始化
    pit_ms_init(CCU61_CH0, 50);     //开启61中断0初始化

    //初始化完成
    Buzzer_check(50);

    // 此处编写用户代码 例如外设初始化代码等
	cpu_wait_event_ready();         // 等待所有核心初始化完毕
	while (TRUE)
	{
        // 此处编写需要循环执行的代码

        Num++;
        ips200_show_int(0 , 16*0, Num, 5);

        /*陀螺仪角度显示*/
        ips200_show_string(0, 16*1, "Yaw:");ips200_show_float(30, 16*1, Yaw, 3, 6);
        ips200_show_string(0, 16*2, "yaw:");ips200_show_float(30, 16*2, yaw, 3, 6);

        //地面航向（000.0~359.9 度，以真北方为参考基准）+
//        ips200_show_string(0, 16*3, "h_x:" ); ips200_show_float(60, 16*3, gnss.direction,4, 6);//
        /*自动控制*/
        Auto_control();
        /*GPS解析*/
        GPS_parse();

        /*在页面上展示时时经纬度*/
        ips200_show_string(0, 16*13, "LAT:");  ips200_show_float(60, 16*13, gnss.latitude, 4, 6);// 纬度
        ips200_show_string(0, 16*14, "LOG:");  ips200_show_float(60, 16*14, gnss.longitude, 4, 6);// 经度
        ips200_show_string(0, 16*15, "number:");ips200_show_uint( 60, 16*15, gnss.satellite_used, 5);  //卫星数量

        /*串口解析：接收数据为整形，串口助手以文本格式发送了，m%dx%dy%dz%d ；其中‘%d’为对应函数参数,能同时解析四个整数*/
        Serial_Data_Resolving(&m,&x,&y,&z);
        x = Filter(x);
        ips200_show_int(0 , 16* 9,   m, 5);
        ips200_show_int(0 , 16*10,   x, 5);
        ips200_show_int(0 , 16*11,   y, 5);
        ips200_show_int(0 , 16*12,   z, 5);

        ips200_show_string(0, 16*17, "Target:");ips200_show_int( 60, 16*17, Target_Position, 5);

        Target_Position = m;
        if(gpio_get_level(sw1) &&  gpio_get_level(sw2))  //1,2下
        {
            if(key2_flag == 1)
            {
                key2_flag=0;
                Buzzer_SetTimer(50);
                delay_Start = 1;    //开启发车1s倒计时
            }
            if(Timer_delay_ms(1000)==1) //不堵塞式等待1s倒计时完成
            {
                dispatch_car = 1;   //发车标志位
                delay_ScanTemp = 0; //清除置1标志位
            }
            if(dispatch_car == 1 && next == 1 && Transform == 1)  //发车起始速度
            {
                Back_Motor_SetPwm(200,200);
            }
        }
        //发车显示
        if(dispatch_car == 1){ips200_show_string(100, 16*8, "  go");}
        else{ips200_show_string(100, 16*8, "Stop");}

        //选择科目
        if(!gpio_get_level(sw1) && gpio_get_level(sw2))  //1上.2下
        {
            if(key1_flag==1)
            {
                key1_flag=0;
                ips200_clear();
                Buzzer_check(50);

                if(Km13 == 0)
                {
                    Km13 = 1;
                    KM_Select_flag = 1;     //选择科目
                }
                else
                {
                    Km13 = 0;
                    KM_Select_flag = 3;     //选择科目
                }

            }
            if(key2_flag==1)
            {
                key2_flag=0;
                KM_Select_flag = 2;     //选择科目
                KM2_Init();
                Buzzer_check(50);
            }
        }
//        KM_Select_flag = 3;//////////////////////

        switch (KM_Select_flag)
        {
            case 1:    //科目一
            {
                ips200_show_string(195, 16*8, "KM1"); // 科目一flag

                    System_Setting_KM1();
                    if(Transform == 1)
                    {
                        Follow_track1();
                    }
            }
            break;
            case 2:
                {
                    ips200_show_string(195, 16*8, "KM2"); //科目二flag

//                    System_NSetting_KM2();
//
//                    if(gpio_get_level(sw1) &&  gpio_get_level(sw2))  //1,2下
//                    {
////                        Send_Voice();
////                        Wifi_Uart_message_parse();
//
//                        if(key4_flag==1)
//                        {
//                            key4_flag=0;
////                            Process_Command(Task_instructions[0]);
////                            Process_Command(Task_instructions[1]);
////                            Process_Command(Task_instructions[2]);
//                            Buzzer_check(50);
//                        }
//                    }

//                    Track2_Km2(Task_instructions[1],Task_instructions[2]);
                }
            break;
            case 3:    //科目3
            {
                ips200_show_string(195, 16*8, "KM3"); // 科目一flag
//
//                if(Transform == 1)
//                {
//                    Follow_track3();
//                }
            }
            break;
        }








//        system_delay_ms(500);
        // 此处编写需要循环执行的代码
	}
}


//IFX_INTERRUPT(cc60_pit_ch0_isr, 0, CCU6_0_CH0_ISR_PRIORITY)
//{
//    interrupt_global_enable(0);                     // 开启中断嵌套
//    pit_clear_flag(CCU60_CH0);
//
//    gpio_toggle_level(LED1);
//    gpio_toggle_level(LED2);
//    gpio_toggle_level(LED3);
//    gpio_toggle_level(LED4);
//
//}

// **************************** PIT中断函数 ****************************
IFX_INTERRUPT(cc60_pit_ch0_isr, 0, CCU6_0_CH0_ISR_PRIORITY)     //中断时间为10ms
{
    interrupt_global_enable(0);                     // 开启中断嵌套
    pit_clear_flag(CCU60_CH0);

    static int8 Time = 0;

//    All_PID_scan_adjust_parameter();    /*速度调参代码，勿删 测试时使用，正常用注释掉*/

    Dir_Motor_Encoder_PID_Update(); /*底层转向PID，勿动*/

    PID_Controller_Update();

    Time++;
    if(Time == 2)
    {
        Time = 0;
        KM3_PID_Controller_Update();
    }
}


IFX_INTERRUPT(cc60_pit_ch1_isr, 0, CCU6_0_CH1_ISR_PRIORITY) //中断时间为1ms
{
    interrupt_global_enable(0);                     // 开启中断嵌套
    pit_clear_flag(CCU60_CH1);

    static int16 T = 0;
    T++;
    if(T>=15)
    {
        T = 0;
        key_Scan(); //按键扫描
    }
    imu660ra_get_acc();         // 新增加速度计采集
    imu660ra_get_gyro();        // 采集陀螺仪数据
    quaternion_update();        //更新四元数
    Get_Angle();                //陀螺仪角度

    Buzzer_scan();              //蜂鸣器扫描
    Timer_delay_scan();         //延迟delay扫描
}

IFX_INTERRUPT(cc61_pit_ch0_isr, 0, CCU6_1_CH0_ISR_PRIORITY)//中断时间为50ms
{
    interrupt_global_enable(0);                     // 开启中断嵌套
    pit_clear_flag(CCU61_CH0);

    LED_Tick();

    Read_Encodernum1 = Read_Encoder(TIM2_ENCODER);
    Read_Encodernum2 = Read_Encoder(TIM5_ENCODER);

}

IFX_INTERRUPT(cc61_pit_ch1_isr, 0, CCU6_1_CH1_ISR_PRIORITY)
{
    interrupt_global_enable(0);                     // 开启中断嵌套
    pit_clear_flag(CCU61_CH1);
//    Voice_Get();

}



#pragma section all restore
// **************************** 代码区域 ****************************

// *************************** 例程常见问题说明 ***************************
// 遇到问题时请按照以下问题检查列表检查
// 问题1：屏幕不显示
//      如果使用主板测试，主板必须要用电池供电 检查屏幕供电引脚电压
//      检查屏幕是不是插错位置了 检查引脚对应关系
//      如果对应引脚都正确 检查一下是否有引脚波形不对 需要有示波器
//      无法完成波形测试则复制一个GPIO例程将屏幕所有IO初始化为GPIO翻转电平 看看是否受控
