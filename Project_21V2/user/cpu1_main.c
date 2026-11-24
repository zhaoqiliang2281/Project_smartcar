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
* 文件名称          cpu1_main
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
#pragma section all "cpu1_dsram"
// 将本语句与#pragma section all restore语句之间的全局变量都放在CPU1的RAM中


// 工程导入到软件之后，应该选中工程然后点击refresh刷新一下之后再编译
// 工程默认设置为关闭优化，可以自己右击工程选择properties->C/C++ Build->Setting
// 然后在右侧的窗口中找到C/C++ Compiler->Optimization->Optimization level处设置优化等级
// 一般默认新建立的工程都会默认开2级优化，因此大家也可以设置为2级优化

// 对于TC系列默认是不支持中断嵌套的，希望支持中断嵌套需要在中断内使用 enableInterrupts(); 来开启中断嵌套
// 简单点说实际上进入中断后TC系列的硬件自动调用了 disableInterrupts(); 来拒绝响应任何的中断，因此需要我们自己手动调用 enableInterrupts(); 来开启中断的响应。


// **************************** 代码区域 ****************************

// ==================== 跨核命令邮箱(定义在 CPU1 DSRAM,CPU0 写/CPU1 执行) ====================
// CPU0 把“本次新增的语音命令”发给 CPU1,CPU1 逐条 execute_command 执行一遍即停
// 注:此前用 flash/voice 两组邮箱按索引分区提取,因 data_store 按到达顺序存储导致鸣笛索引错位、
//     灯光被重放,故改为通用增量邮箱,灯光/鸣笛/运动命令统一由 execute_command 区分处理
volatile uint8 mbox_ready = 0;                  // 1 = CPU0 已提交新命令
volatile uint8 mbox_done  = 1;                  // 1 = CPU1 执行完毕(初始空闲)
uint8  mbox_cmd[DATA_STORE_MAX];                // 本次要执行的命令序列(最大64条)
uint32 mbox_sum;                                // 本次要执行的命令条数

void core1_main(void)
{
    disable_Watchdog();                     // 关闭看门狗
    interrupt_global_enable(0);             // 打开全局中断
    // 此处编写用户代码 例如外设初始化代码等




    // 此处编写用户代码 例如外设初始化代码等
    cpu_wait_event_ready();                 // 等待所有核心初始化完毕
    while (TRUE)
    {
        // 此处编写需要循环执行的代码

        // ==================== CPU1 执行跨核命令(灯光/鸣笛逐条执行,运动命令在 execute_command 内为空操作) ====================
        if (mbox_ready)                     // CPU0 有新的命令
        {
            uint32 i;
            mbox_ready = 0;
            for (i = 0; i < mbox_sum; i++)  // 只执行本次新增的命令,执行完即停
            {
                execute_command(mbox_cmd[i]);
            }
            __dsync();                      // 数据同步屏障:确保执行结果对外可见
            mbox_done = 1;                  // 通知 CPU0 本次命令执行完毕
        }

        // 此处编写需要循环执行的代码
    }
}


#pragma section all restore
