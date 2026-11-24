/*
 * nagivation.h
 *
 *  Created on: 2024年10月16日
 *      Author: Monst
 */

#ifndef _NAVIGATION_H_
#define _NAVIGATION_H_


#define Nag_Set_mileage 1500   // 里程计间隔(约50cm)
#define NagPoint_MAX 500       // Flash最大点数

#define wheel_radius (5.0f) // 车轮半径


extern int32 Last_Encoder;
extern int32 Nag_Point_Flag;
extern int32 Nag_ResdPoint_Flag;

extern int32 Nag_data[NagPoint_MAX];
extern int32 Nag_data2[NagPoint_MAX];
extern int32 delta;

extern int32 Last_Encoder_Read;
extern int32 delta2;

extern int32 Run_Index;
extern float Nag_Target_Yaw;
extern int8  Nag_Target_Dir;

extern int32 INS_Go_Point_Flag;
extern int32 INS_Back_Point_Flag;

extern int32 Read_Continue_Encoder_Left;


void Nag_write(void);
void Nag_FlashWrite(void);
void Nag_FlashRead(void);
void Nag_Decode(int32 raw, float *yaw, int8 *dir);
void Nag_Read(void);
void Nag_ReversePath(void);

/************************************************
 *  备   注：以下为第二次校赛之后新方案，用编码器及陀螺仪构建二维坐标，GPS定时校准
 ************************************************/
#define PATH_MAX 500
#define WHEEL_C         0.77f
#define ENCODER_PPR     1024.0f
#define RECORD_DIS      0.5f
#define INS_distance 1.5f
#define INS_distance_final 0.2f

extern float INS_X;
extern float INS_Y;
extern int32 INS_Resd_GoPoint_Flag;
extern int32 INS_Resd_BackPoint_Flag;

void INS_Update(void);
void System_INS(void);
void INS_choise(void);
void reverse_path_xy(int32 *path, int32 len);

// ==== GPS组合导航新增 ====
extern float GPS_X;
extern float GPS_Y;
extern uint8 gps_origin_set;
void GPS_ToXY(void);
void GPS_SetOrigin(void);

// ==== GPS纯坐标路点 (Page 10) ====
extern int32 GPS_xy_data[NagPoint_MAX];
extern int32 GPS_xy_data2[NagPoint_MAX];
extern int32 INS_data2[NagPoint_MAX];
extern int32 GPS_Go_Point_Flag;
extern int32 GPS_Back_Point_Flag;
extern int32 GPS_Resd_GoPoint_Flag;
extern int32 GPS_Resd_BackPoint_Flag;
extern uint8 INS_Flag;
void GPS_FlashWrite(void);
void GPS_FlashRead(void);
void GPS_Correct_INS(void);
void INS_Correct_GPS(void);
void GPS_Replace_INS(void);
void INS_GPS_Display(void);

// ==== 旋转变码器逐点编辑 ====
#define WP_EDIT_STEP    0.10f       // 编辑器步长(10cm)
void Waypoint_Editor(void);
void KM2_WaypointEditor(void);   /* KM2 GPS路点编辑器, 模拟KM3修点模式 */

float Get_Two_Point_Distance(float x1, float y1, float x2, float y2);
float Get_Target_Angle(float x1, float y1, float x2, float y2);

// ==== KM3 菜单动作函数 ====
void KM3_Record(void);
void KM1_SaveFlash(void);
void KM1_LoadFlash(void);
void KM3_SaveFlash(void);
void KM3_LoadFlash(void);
void KM3_Run(void);
void KM3_Stop(void);
void KM3_ReversePath(void);
void KM3_DisplayPath(void);
void KM3_SpeedSet(void);
void KM3_Correct(void);
void KM3_Replace(void);
void KM3_Editor(void);
void KM3_Gain(void);
void KM3_GPSMode(void);

extern float gps_fusion_gain;
extern uint8 gps_confirm_mode;

// ==== 差速转向 ====
extern float diff_gain;        // 差速增益 (0.0=关闭, ~0.5=最大, 默认0.35)
extern int32 km3_fwd_speed;    // KM3前进速度 (默认500)
extern int32 km3_rev_speed;    // KM3后退速度 (默认250)
void Back_Motor_SetPwm_Diff(int32 base_speed, float angle_error);

#endif /* _NAVIGATION_H_ */
