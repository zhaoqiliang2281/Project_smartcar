/*
 * subject.h
 *
 *  Created on: 2025年6月15日
 *      Author: LiangZhaoqi
 */

#ifndef CODE_SUBJECT_H_
#define CODE_SUBJECT_H_

#define need_distance    2.2          //1.8(K2)

extern int16 next;

extern int8 Back_Motor_PID_Start_flag;
extern double home_run[Point_MAX];

extern int32 Speed_KM2_Buf[16];//科目2速度限幅

void System_Setting_KM1(void);
void System_Setting_KM2(void);
void System_Setting_KM3(void);

void Follow_track1(void);//科目一
void Follow_track2(void);//科目二
void Follow_track3(void);//科目三

void KM_Speed_Change(uint8 KM);

extern int32 Back_Motor_Set_Speed;

#endif /* CODE_SUBJECT_H_ */
