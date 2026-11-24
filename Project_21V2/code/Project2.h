/*
 * Project2.h
 *
 *  Created on: 2026年3月12日
 *      Author: 耿子卓
 */

#ifndef CODE_PROJECT2_H_
#define CODE_PROJECT2_H_



#include "zf_common_headfile.h"


#define PULSE_PER_TURN    1024    // 一圈1024脉冲
#define WHEEL_RADIUS      126      // 轮子半径，单位：mm


extern uint8 Km2_protect_flag;

extern uint8 dispatch_car;

extern uint8 MenDong_Flag;
extern uint8 MenDong_Flag2;
extern uint8 Go_Flag;
extern uint8 Back_Flag;

extern uint8 Go_Num,Back_Num;

extern float Moving_Distance;

extern double LAT_error,LOG_error;

void System_NSetting_KM2(void);
uint8 Track2_Km2(uint8 Task,uint8 Task2);
void IMU_Control(double Expectation_Yaw);
void text_point(void);

uint8 Go_straight(void);
uint8 Back_straight(void);
uint8 Left_straight(void);
uint8 Right_straight(void);
uint8 Left_circlet(void);
uint8 Right_circlet(void);
uint8 Go_S(void);
uint8 Back_S(void);
uint8 Sport_choise(uint8 sprot_num[],uint8 sport_sum,uint8 first_num);
uint8 Flash_choise(uint8 Flash_num[],uint8 Flash_sum,uint8 first_num);
uint8 Voice_choise(uint8 Voice_num[],uint8 Voice_sum,uint8 first_num);


// ==================== 跨核任务接口(灯光/鸣笛命令交由 CPU1 执行) ====================
// CPU0 调用下面的提交函数,把“本次新增的语音命令”发给 CPU1,CPU1 逐条执行一遍即停
uint8 cmd_task_submit(uint8 cmd[], uint32 sum);

// 跨核共享邮箱变量(定义在 cpu1_main.c 的 cpu1_dsram 段内,CPU0 通过它们与 CPU1 同步)
extern volatile uint8 mbox_ready;
extern volatile uint8 mbox_done;
extern uint8  mbox_cmd[];
extern uint32 mbox_sum;

#endif /* CODE_PROJECT2_H_ */
