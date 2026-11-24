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

// **************************** 代码区域 ****************************

double Temp;

uint32_t image_time = 0;
float KP = 1.0;
float KD = 0.0;

uint8 flag_control = 0;
int8 flag_go = 1;   //科目标志位
uint8 dispatch_car = 0; //发车标志位

void core0_main(void)
{
    clock_init();                   // 获取时钟频率<务必保留>
    debug_init();                   // 初始化默认调试串口
    // 此处编写用户代码 例如外设初始化代码等        ips200_draw_line(188, 120, 0, 120, RGB565_WHITE);

    ALL_init();
//    mt9v03x_init();//摄像头初始化
//    image_data_init();

    // 此处编写用户代码 例如外设初始化代码等
    cpu_wait_event_ready();         // 等待所有核心初始化完毕
    while (TRUE)
    {
        // 此处编写需要循环执行的代码

        /*陀螺仪角度显示*/
        ips200_show_float(0, 16*11, Yaw, 3, 6);
        Servo_camera_PID_Init(KP, KD);

        /*遥控接收数据显示*/
//        HotRC_receive_data_Show();

        /*GPS解析*/
        GPS_parse();

        /*在页面上展示时时经纬度*/
        ips200_show_string(0, 16*13, "LAT:");  ips200_show_float(60, 16*13, gnss.latitude, 4, 6);// 纬度
        ips200_show_string(0, 16*14, "LOG:");  ips200_show_float(60, 16*14, gnss.longitude, 4, 6);// 经度
        ips200_show_string(0, 16*15, "number:");ips200_show_uint( 60, 16*15, gnss.satellite_used, 5);  //卫星数量
        ips200_show_uint( 60, 16*16, dispatch_car, 5);

        if(!gpio_get_level(sw1) &&  !gpio_get_level(sw2))//1,2上
        {
            if(key5_flag == 1)
            {
                key5_flag=0;
                flag_go++;
                if(flag_go>3){flag_go=1;}
                Buzzer_SetTimer(50);
            }
        }
        if(gpio_get_level(sw1) &&  !gpio_get_level(sw2))  //1下.2上
        {
            //修改点位
            Key_Change_LAT_LON();
        }

        if(gpio_get_level(sw1) &&  gpio_get_level(sw2))  //1,2下
        {
            if(key2_flag == 1)
            {
                key2_flag=0;
                Buzzer_SetTimer(50);
                delay_Start = 1;
            }
            if(Timer_delay_ms(1000)==1)
            {
                dispatch_car = 1;//发车标志位
                Speed_duty=1;  //舵机pid开启标志位
            }
            if(dispatch_car == 1 && next == 0)  //发车起始速度
            {
                Pwm_Motor(20);
            }

        }
        if(dispatch_car == 1)
        {
            ips200_show_string(100, 16*8, "  go");
        }
        else
        {
            ips200_show_string(100, 16*8, "Stop");
        }
        switch (flag_go)
        {
            case 1:    //科目一
            {
                ips200_show_string(195, 16*8, "KM1"); // 科目一flag
                System_Setting_KM1();
                Follow_track1();
            }
                break;
            case 2:     //科目二
            {
                ips200_show_string(195, 16*8, "KM2"); //科目二flag
                System_Setting_KM2();
                Follow_track2();
        /***************科二选择速度大小**********************/
               if(key3_flag == 1 && gpio_get_level(sw1) &&  gpio_get_level(sw2))
               {
                   key3_flag=0;
                   for(int i = 0;i<14;i++ )
                   {
                       Speed_km2_Buf[i] = Speed_km2_1_Buf[i];
                   }
                   ips200_show_string(195, 16*7, "max");
                   Buzzer_SetTimer(50);
               }
               if(key4_flag == 1 && gpio_get_level(sw1) &&  gpio_get_level(sw2))
               {
                   key4_flag=0;
                   for(int i = 0;i<14;i++ )
                   {
                       Speed_km2_Buf[i] = Speed_km2_2_Buf[i];
                   }
                   ips200_show_string(195, 16*7, "min");
                   Buzzer_SetTimer(50);
               }
        /**********************************************/
            }
                break;
            case 3:     //科目三
            {
                ips200_show_string(195, 16*8, "KM3"); //科目三flag
                System_Setting_KM3();
                Follow_track3();
                /***************科三元素位置选择**********************/
                if(key3_flag == 1 && gpio_get_level(sw1) &&  gpio_get_level(sw2))
                {
                   key3_flag=0;
                   KM3weizixuanze=0;
                   ips200_show_string(195, 16*7, "po ");
                   Buzzer_SetTimer(50);
                }
                if(key4_flag == 1 && gpio_get_level(sw1) &&  gpio_get_level(sw2))
                {
                   key4_flag=0;
                   KM3weizixuanze=1;
                   ips200_show_string(195, 16*7, "do ");
                   Buzzer_SetTimer(50);
                }
                /**********************************************/
            }
                break;
        }

/*=========================下面是测试代码===================================*/
        /*速度调参代码，勿删 测试时使用，正常用注释掉*/
//        PID_DataShow();
//        PID_adjust_parameter();

        //遥控控制车代码
//        HotRC_control();

        /*摄像头*/
//        ImageProcess();
//        ips200_show_gray_image(0, 0, display_image[0], LCDW, LCDH, MT9V03X_W, MT9V03X_H,0);
//        ips200_show_int( 60, 16*18, offect, 5);

//        if(!gpio_get_level(sw1) &&  gpio_get_level(sw2))  //1上.2下
//        {
//            KM_Speed_Change(flag_go);
//        }
/*=========================上面是测试代码===================================*/
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

    //PID计算
    All_PID_scan();

//    All_PID_scan_adjust_parameter();    /*速度调参代码，勿删 测试时使用，正常用注释掉*/

    //按键扫描
    key_scan();
}


IFX_INTERRUPT(cc60_pit_ch1_isr, 0, CCU6_0_CH1_ISR_PRIORITY) //1ms
{
    interrupt_global_enable(0);                     // 开启中断嵌套
    pit_clear_flag(CCU60_CH1);

    imu660ra_get_acc();         // 新增加速度计采集
    imu660ra_get_gyro();        // 采集陀螺仪数据
    quaternion_update();        //更新四元数
    Get_Angle();                //陀螺仪角度

    Buzzer_scan();              //蜂鸣器扫描
    Timer_delay_scan();         //延迟delay扫描
}

IFX_INTERRUPT(cc61_pit_ch0_isr, 0, CCU6_1_CH0_ISR_PRIORITY)
{
    interrupt_global_enable(0);                     // 开启中断嵌套
    pit_clear_flag(CCU61_CH0);

//    image_PID(); /*速度调参代码，勿删 测试时使用，正常用注释掉*/
//    if(view_flag == 2)
//    {
//        image_time++;
//        image_PID();
//        if(image_time == 1000)
//        {
//            view_flag = 0;
//        }
//    }

}
