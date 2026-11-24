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
/*

Debug/VOFA   : 串口号：UART_0  TX 引脚：UART0_TX_P14_0   RX 引脚：UART0_RX_P14_1
摄像头SCC8660: 串口号：UART_1  TX 引脚：UART1_TX_P02_2   RX 引脚：UART1_RX_P02_3
无线/WIFI/SBUS: 串口号：UART_2  TX 引脚：UART2_TX_P10_5   RX 引脚：UART2_RX_P10_6
GPS/GNSS     : 串口号：UART_3  TX 引脚：UART3_TX_P15_6   RX 引脚：UART3_RX_P15_7
TLD7002灯光  : 串口号：UART_4  TX 引脚：UART4_TX_P00_9   RX 引脚：UART4_RX_P00_12
CI1302       : 串口号：UART_9  TX 引脚：UART9_TX_P20_7   RX 引脚：UART9_RX_P20_6
BY9301       : 串口号：UART_10 TX 引脚：UART10_TX_P00_8  RX 引脚：UART10_RX_P13_1

*/

#include "zf_common_headfile.h"
#pragma section all "cpu0_dsram"
// 将变量分配到CPU0的DSRAM中，使用#pragma section all restore恢复

// ==================== 全局变量定义区 ====================

// ----- 发车/科目选择相关 -----
uint8 dispatch_car = 0;      // 发车标志位：1=发车（电机启动），0=停车。科目完成后自动置0


int Num;                      // 系统运行计数器，每循环+1，显示在屏幕第一行

// ----- 编码器读数 -----
int16 Read_Encoder_Left, Read_Encoder_Right;       // 左右编码器原始读数（单次）
int32 Read_Encoder_Left1, Read_Encoder_Right1;      // 左右编码器累计读数


// ----- 调试/测试变量 -----
double yaw_test = 0;           // 航向角测试值
int8  dir_test  = 0;           // 方向测试值

// ----- 跨核提交缓存:记录上次已提交的命令数,有新命令时只提交增量,保证 CPU1 只执行一遍 -----
uint32 last_submit_count = 0;            // 上次已提交的命令总数(初始0)
uint8  task_pending      = 0;            // 有待提交的新命令(置1后等 CPU1 空闲才提交一次)

uint8 while_flag = 1;

int core0_main(void)
{
    // ==================== 系统初始化 ====================
    clock_init();                   // 获取系统时钟频率 <时钟初始化>
    debug_init();                   // 初始化默认调试串口

    ALL_init();                     // 【总初始化】蜂鸣器/LED/按键/屏幕/IMU/GPS/电机/编码器/PID等全部外设

    blue_square_camera_init();// 摄像头初始化
//    vision_init();   //+ 屏幕欢迎信息 (Color_discirmination.c)

    // ==================== 定时器中断配置 ====================
    pit_ms_init(CCU60_CH0, 10);     // 核60中断0，10ms周期 -> PID控制器更新 + LED闪烁
    pit_ms_init(CCU60_CH1, 1);      // 核60中断1，1ms周期 -> 按键扫描(15ms一次)/陀螺仪角度/蜂鸣器/延时
    pit_ms_init(CCU61_CH0, 500);    // 核61中断0，500ms周期 -> 预留
    pit_ms_init(CCU61_CH1, 10);     // 核61中断1，10ms周期 -> INS惯导更新

    // ==================== 启动提示 ====================
    Buzzer_check(50);               // 蜂鸣器响50ms，提示系统启动完成
    ips200_clear();                 // 清屏

    ips200_show_chinese(3 * 8, 16 * 1, 32, (const uint8 *)Electric_Innovation32, 6, RGB565_GREEN); // 显示汉字
    system_delay_ms(10);
    ips200_show_rgb565_image(0, 80, (const uint16 *)gImage_c, 240, 240, 240, 240, 0);
    system_delay_ms(10);

    // 在此处添加用户代码
    cpu_wait_event_ready();         // 等待其他中断服务程序准备就绪
    while (TRUE)
    {
        /**********************保护锁*****************************************/
        while(while_flag)
        {
            static uint8 sum=0,a=0,b=0;

            if(key1_flag == 1)
            {
                key1_flag=0;
                Buzzer_check(50);
                b+=2;
            }
            if(key2_flag == 1)
            {
                a++;
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

        /**********************保护锁*****************************************/

        // 在此处添加需要循环执行的代码

//        Num++;
//        ips200_show_int(10*16 , 16*17, Num, 2);// 显示系统运行计数器
        ips200_show_float(0 , 16*10, Yaw_Mapped, 3,6);// 显示映射航向角

        blue_square_camera_process();

        System_INS();
        INS_choise();

        CI1302_process_fifo_data();    //解析离线语音模块传输的数据并显示

        /*==================== 跨核版本:灯光/鸣笛命令  交由 CPU1 执行,CPU0 不再阻塞 ==================== */
        /* 有新命令(命令总数增加)先置 pending;等 CPU1 空闲时只提交【新增的命令】,不重放历史 */
        if(data_store_count != last_submit_count)
        {
            task_pending = 1;                   // 有新的未处理命令
        }

        if(task_pending && mbox_done)           // CPU1 空闲才提交
        {
            cmd_task_submit(&data_store[last_submit_count], data_store_count - last_submit_count);  // 只发新增命令
            last_submit_count = data_store_count;   // 记录已提交的命令数
            task_pending      = 0;              // 清 pending,防止本批重复提交
        }

        /* 自动/手动控制检测 */
        Auto_control();          // 检测KEY6(脚踏)/KEY7(自动手动切换)/KEY8(方向切换)

    }
}

#pragma section all restore

// **************************** PIT中断函数 ****************************

/*
 * CH0 中断 (10ms 周期):
 *   该中断函数由定时器 pit_ms_init(CCU60_CH0, 10) 配置而来
 *
 *   几点说明：
 *     1. 按键扫描移入中断确保 10ms 定时精度，
 *     2. 所有模式下(MENU/KM1/KM2/KM3/DIPSW)都需要响应 K1 退出，所以统一扫描，
 *        避免在各个 case 中重复实现。
 *     3. 中断提供硬实时保证 -> 即使主循环被其他任务阻塞，也不会丢失时间
 *
 *   注意：旋转编码器驱动（P13_2, P21_6）扫描在 menu_encoder_update()
 *   中完成，模式切换由主循环处理，所以此处只负责按键状态更新。
 */
IFX_INTERRUPT(cc60_pit_ch0_isr, 0, CCU6_0_CH0_ISR_PRIORITY)     //中断周期为10ms
{
    static uint32 sys_ms = 0;          // 10ms 累加计时（双击检测用）

    interrupt_global_enable(0);                     // 开启中断嵌套
    pit_clear_flag(CCU60_CH0);
    sys_ms += 10;                       // 每次中断 +10ms

    PID_Controller_Update();        /*PID更新*/

    menu_key_scanner();

    /* [新增] 编码器按键短按/长按锁存 -> 由 System_INS 消费 */
    if (MENU_KEY_SHORT_PRESS == menu_key_get_state(MENU_KEY_1))
    {
        menu_key_clear_state(MENU_KEY_1);
        Encoder_Btn_Short = 1;
    }
    else if (MENU_KEY_LONG_PRESS == menu_key_get_state(MENU_KEY_1))
    {
        menu_key_clear_state(MENU_KEY_1);
        Encoder_Btn_Long = 1;
    }             /* 菜单按键状态扫描: 每帧 + 按键抖动处理 (所有模式统一) */

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

    // ---- 1ms更新Yaw映射 ----
    Yaw_Mapped = Yaw + yaw_offset;
    if(Yaw_Mapped >= 360.0) Yaw_Mapped -= 360.0;
    if(Yaw_Mapped <    0.0) Yaw_Mapped += 360.0;

    Buzzer_scan();              //蜂鸣器扫描
    Timer_delay_scan();         //软件delay扫描
}

IFX_INTERRUPT(cc61_pit_ch0_isr, 0, CCU6_1_CH0_ISR_PRIORITY)//中断时间为50ms
{
    interrupt_global_enable(0);                     // 开启中断嵌套
    pit_clear_flag(CCU61_CH0);

    LED_Tick();
}

IFX_INTERRUPT(cc61_pit_ch1_isr, 0, CCU6_1_CH1_ISR_PRIORITY)//中断周期为10ms
{
    interrupt_global_enable(0);                     // 开启中断嵌套
    pit_clear_flag(CCU61_CH1);

    INS_Update();       //INS惯导更新

}




// **************************** 文件结束 ****************************
