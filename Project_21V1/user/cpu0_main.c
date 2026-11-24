/*********************************************************************************************************************
* TC387 Opensourec Library 即TC387 开源库，是一个官方 SDK 接口的驱动程序开源库
* Copyright (c) 2022 SEEKFREE 科技公司
*
* 本文件为 TC387 开源库提供支持
*
* TC387 开源库 由英飞凌公司提供
* 本库遵循 GPL（GNU General Public License）协议，GNU通用公共许可证的条款
* 如果使用 GPL 协议的第3版本（即 GPL3.0），本库不会对使用该软件造成的任何后果负责
*
* 如果您使用本库进行商业活动，但不想公开您的源代码
* 您可以联系 SEEKFREE 公司购买商业许可证
* 具体详情请参考 GPL 协议
*
* 您应该已经收到本库附带的 GPL 协议副本
* 如果没有，请访问 <https://www.gnu.org/licenses/>
*
* 特别声明：
* 如果您使用 GPL3.0 协议进行开发，那么您的代码也必须开源
* 具体说明请查看 libraries/doc 目录下的 GPL3_permission_statement.txt 文件
* 如果 libraries 目录下没有该文件，请查看 LICENSE 文件
* 欢迎使用本库，如果您在使用过程中遇到任何问题，请随时联系 SEEKFREE 公司获取技术支持
*
* 文件名                cpu0_main
* 公司名称              SEEKFREE 科技公司
* 版本信息              请查看 libraries/doc 目录下的 version 文件 版本说明
* 编译器                ADS v1.9.4
* 单片机平台            TC387QP
* 淘宝店铺              https://seekfree.taobao.com/
*
* 修改记录
* 日期                作者                内容
* 2022-11-04       pudding            first version
********************************************************************************************************************/
#include "zf_common_headfile.h"
#pragma section all "cpu0_dsram"
// 将变量分配到CPU0的DSRAM中，使用#pragma section all restore恢复

// **************************** 全局变量 ****************************

uint8 dispatch_car = 0; //转向标志位
int Num;                //系统计数
uint8 Km13 = 0;

int core0_main(void)
{
    clock_init();                   // 获取时钟频率<必须调用>
    debug_init();                   // 初始化默认调试通道
    // 在此处添加用户代码

    //所有基础初始化
    ALL_init();

    //开启定时器中断
    pit_ms_init(CCU60_CH0, 10);     //定时器60中断0初始化
    pit_ms_init(CCU60_CH1, 1);      //定时器60中断1初始化
    pit_ms_init(CCU61_CH0, 50);     //定时器61中断0初始化
    pit_ms_init(CCU61_CH1,10);      //定时器61中断1初始化
    //初始化蜂鸣器
    Buzzer_check(50);
    ips200_clear();
    // 在此处添加用户代码
    cpu_wait_event_ready();         // 等待其他中断服务程序准备就绪

    while (TRUE)
    {
        // 在此处添加需要循环执行的代码
        Num++;
        ips200_show_int(120 , 16*13, Num, 5);

        /*自动控制*/
        Auto_control();
        /*GPS数据解析*/
        GPS_parse();

        /*当前页面展示实时经纬度*/
        if(KM_Select_flag != 3 || INS_Flag == 2)
        {
            /* ---- 右侧数据栏 (x=120起, 不遮挡左侧菜单) ---- */
            ips200_show_string(90, 16*0, "LAT:"); ips200_show_float(120, 16*0, gnss.latitude, 3, 6);
            ips200_show_string(90, 16*1, "LOG:"); ips200_show_float(120, 16*1, gnss.longitude, 3, 6);
            ips200_show_string(120, 16*15, "YAW:"); ips200_show_float(155, 16*15, Yaw, 3, 2);
            ips200_show_string(120, 16*16, "ECD:"); ips200_show_int(155, 16*16, Read_Encoder_Aggregate_Value(TIM5_ENCODER), 6);

            /* GPS/INS坐标: 上排INS, 下排GPS */
            ips200_show_string(120, 16*9,  "IX:"); ips200_show_float(145, 16*9,  INS_X, 3, 2);
            ips200_show_string(120, 16*10, "IY:"); ips200_show_float(145, 16*10, INS_Y, 3, 2);
            ips200_show_string(120, 16*11, "GX:"); ips200_show_float(145, 16*11, GPS_X, 3, 2);
            ips200_show_string(120, 16*12, "GY:"); ips200_show_float(145, 16*12, GPS_Y, 3, 2);
            if(gps_origin_set)
                ips200_show_string(120, 16*8, "GPS OK");
            else
                ips200_show_string(120, 16*8, "GPS --");
        }

        static uint8 menu_inited = 0;
        if(!menu_inited)
        {
            menu_init(&main_menu);
            menu_inited = 1;
        }

        /* ---- 系统模式调度 ---- */
        {
            static uint8 last_mode = 0;
            if(menu_system_mode != last_mode) { ips200_full(RGB565_BLACK); last_mode = menu_system_mode; }
        }
        switch(menu_system_mode)
        {
            case 0: // 菜单模式
                menu_task();
                INS_choise();
                break;

            case 1: // KM1 模式 (科目一: INS, 同KM3)
                menu_key_scanner();
                if(MENU_KEY_LONG_PRESS == menu_key_get_state(MENU_KEY_1))
                {
                    menu_key_clear_state(MENU_KEY_1);
                    KM3_Stop();
                }
                INS_choise();
                break;

            case 2: // KM2 模式 (科目二: 灯光语音)
                menu_key_scanner();
                if(MENU_KEY_LONG_PRESS == menu_key_get_state(MENU_KEY_1))
                {
                    menu_handle_long_press_exit();
                }
                else
                {
                    CI1302_process_fifo_data();       /* 始终处理语音指令 */
                    if(KM2_edit_active)
                    {
                        KM2_WaypointEditor();         /* GPS路点编辑器 */
                    }
                    else
                    {
                        KM2_KeyHandler();             /* 菜单按键处理(不用拨码) */
                    }
                    KM2_StateMachine();               /* 公共状态机 */
                }
                break;

            case 3: // KM3/INS 模式 (科目三: 惯导)
                menu_key_scanner();
                if(MENU_KEY_LONG_PRESS == menu_key_get_state(MENU_KEY_1))
                {
                    menu_key_clear_state(MENU_KEY_1);
                    KM3_Stop();
                }
                INS_choise();
                break;

            case 4: // 拨码开关模式
                menu_key_scanner();
                menu_encoder_update();
                dip_switch_mode_task();
                break;
        }


    }
}

// **************************** PIT中断函数 ****************************
IFX_INTERRUPT(cc60_pit_ch0_isr, 0, CCU6_0_CH0_ISR_PRIORITY)     //中断周期为10ms
{
    static uint16 T1 = 0;
    interrupt_global_enable(0);                     // 开启中断嵌套
    pit_clear_flag(CCU60_CH0);

//    All_PID_scan_adjust_parameter();    /*速度调试值，可删 调试时使用，不用时注释掉*/

    PID_Controller_Update();        /*PID更新，无*/

    T1++;
    if(T1==10)
    {
        LED_Tick();
    }

}


IFX_INTERRUPT(cc60_pit_ch1_isr, 0, CCU6_0_CH1_ISR_PRIORITY) //中断周期为1ms
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

    Get_Angle();                //获取角度值

    Buzzer_scan();              //蜂鸣器扫描
    Timer_delay_scan();         //软件delay扫描
}



IFX_INTERRUPT(cc61_pit_ch1_isr, 0, CCU6_1_CH1_ISR_PRIORITY) //中断周期为10ms
{
    interrupt_global_enable(0);                     // 开启中断嵌套
    pit_clear_flag(CCU61_CH1);

    INS_Update();   //惯导里程更新

}

IFX_INTERRUPT(cc61_pit_ch0_isr, 0, CCU6_1_CH0_ISR_PRIORITY)//中断时间为50ms
{
    interrupt_global_enable(0);                     // 开启中断嵌套
    pit_clear_flag(CCU61_CH0);

}

#pragma section all restore
// **************************** 文件结束 ****************************
