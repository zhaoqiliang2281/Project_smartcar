/*
 * subject.h
 *
 *  Created on: 2025年6月15日
 *      Author: LiangZhaoqi
 */

#ifndef CODE_SUBJECT_H_
#define CODE_SUBJECT_H_

#define need_distance    2.0          //2.15

#define need_distance3     1.5
extern int16 next;

extern int8 Speed_duty; //电机速度
extern int view_flag;

extern int8 flag_speed;

extern uint8 KM3weizixuanze;

extern uint8 Speed_km2_Buf[14];//科目2速度限幅
extern uint8 Speed_km2_1_Buf[14];//科目2速度限幅
extern uint8 Speed_km2_2_Buf[14];

void System_Setting_KM1(void);
void System_Setting_KM2(void);
void System_Setting_KM3(void);

void Follow_track1(void);//科目一
void Follow_track2(void);//科目二
void Follow_track3(void);//科目三

void KM_Speed_Change(uint8 KM);

#endif /* CODE_SUBJECT_H_ */
