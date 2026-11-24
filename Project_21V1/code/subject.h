/*
 * subject.h
 *
 *  Created on: 2025年6月15日
 *      Author: LiangZhaoqi
 */

#ifndef CODE_SUBJECT_H_
#define CODE_SUBJECT_H_

#define need_distance    1.5          //1.8(K2)

extern int16 next;

extern int8 Back_Motor_PID_Start_flag;
extern double home_run[Point_MAX];

extern float KM2_INS_X[16];
extern float KM2_INS_Y[16];

extern int32 Speed_KM1_Buf[15];//科目1速度限幅
extern int32 Speed_KM2_Buf[16];//科目2速度限幅


void reset_gps_index(void);
void gps_index_undo(void);
int16 get_gps_index(void);
const char *km2_pt_name(int16 idx);

void System_Setting_KM1(void);
void System_Setting_KM2(void);
void System_Setting_KM3(void);

void Follow_track1(void);//科目一
void Follow_track2(void);//科目二
void Follow_track3(void);//科目三

void KM_Speed_Change(uint8 KM);

extern int32 Back_Motor_Set_Speed;

#endif /* CODE_SUBJECT_H_ */
