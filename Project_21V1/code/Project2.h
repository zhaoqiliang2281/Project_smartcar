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
#define WHEEL_RADIUS      126      // 车轮半径单位为mm


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
void KM2_StateMachine(void);       /* 公共状态机, menu和dip_switch共用 */
void KM2_KeyHandler(void);         /* 菜单按键处理, 不依赖拨码开关 */
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

void KM2_ActionTest(void);

void KM2_NavMode(void);
void KM2_EditSequence(void);

#endif /* CODE_PROJECT2_H_ */
