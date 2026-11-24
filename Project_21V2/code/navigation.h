/*
 * nagivation.h
 *
 *  Created on: 2024年10月16日
 *      Author: Monst
 */

#ifndef _NAVIGATION_H_
#define _NAVIGATION_H_


#define Nag_Set_mileage 1500   // 里程计间隔（约50cm）
#define NagPoint_MAX 500       // 写入flash的最大点数

#define wheel_radius (5.0f)    // 车轮半径

extern uint8 Camera_Navigation_Motor_flag; //摄像头与惯导切换电机控制标志位

extern int32 Last_Encoder;
extern int32 Nag_Point_Flag;       // 记录路径点数量的标志位
extern int32 Nag_ResdPoint_Flag;   // 读取存储路径点数量的标志位

extern int32 Nag_data[NagPoint_MAX];    // 记录的原始数据数组
extern int32 Nag_data2[NagPoint_MAX];   // 读取的原始数据数组
extern int32 delta;                     // 当前编码器值-上一次编码器值(记录)

extern int32 Last_Encoder_Read;
extern int32 delta2;                    // 当前编码器值-上一次编码器值(读取)

extern int32 Run_Index;                 // 当前段索引值
extern float Nag_Target_Yaw;            // 目标航向角
extern int8  Nag_Target_Dir;



//extern int32 Read_Encoder_Left,Read_Encoder_Right;
extern int32 Read_Continue_Encoder_Left;


void Nag_write(void);
void Nag_FlashWrite(void);
void Nag_FlashRead(void);
void Nag_Decode(int32 raw, float *yaw, int8 *dir);
void Nag_Read(void);
void Nag_ReversePath(void);

/************************************************
 *  注：第二版本校赛之后的改进，用编码器及陀螺仪构建二维坐标，GPS定时校准
 ************************************************/
#define PATH_MAX 500             // 最多存储500个路径点
#define WHEEL_C         0.77f    // 轮胎周长(m)
#define ENCODER_PPR     1024.0f  // 编码器转一圈脉冲数
#define RECORD_DIS      0.5f     // 50cm记录一个点
#define INS_distance    0.8f     // 循迹距离判断，单位为m //1.2
#define INS_distance_final 0.5f  // 循迹最终距离判断，单位为m //1.2

#define LAST_POINT_Y_OFFSET_CM  0.0f  // 最后一个点Y坐标偏移量（厘米，切记单位是cm不是m），正值向Y正方向偏移
#define LAST_POINT_X_OFFSET_CM  -55.0f  // 最后一个点Y坐标偏移量（厘米，切记单位是cm不是m），正值向Y正方向偏移

extern float INS_X;              // X坐标(m)
extern float INS_Y;              // Y坐标(m)
extern float INS_X_MH;          // 前进目标X坐标(m)
extern float INS_Y_MH;          // 前进目标Y坐标(m)
extern uint8 INS_mh_phase;      // MH阶段：0=前进到目标，1=返回原点
extern int32 INS_Resd_Point_Count;       // 读取存储路径点总数

// ---- Yaw映射 ----
extern double Yaw_Mapped;        // 映射后的航向角（0~360°）
extern float yaw_offset;         // 陀螺仪到映射的偏移量
void Set_Map_Yaw(float target);  // 设定映射目标角度

// 6条路径支持
#define MAX_ROUTES 6           // 最大支持路径数
#define FLASH_SECTOR 0          // Flash扇区为0
#define FLASH_PAGE_BASE 9       // 起始页索引（每条路径占1页）

extern int32 current_route_index;  // 当前路径索引(0-5)

void Nag_SwitchRoute(void);    // 切换下一条路径

void INS_Update(void);           // 更新惯导
void System_INS(void);           // 系统惯导
void INS_choise(void);           // 惯导选择
void reverse_path_xy(int32 *path, int32 len); // 路径反转
void INS_OneKeyReturn(void);     // 一键返回
void INS_track_oneway(void);     // 单向循迹
void INS_MH_Return(void);        // 前进到目标点再返回原点

#define INS_EDIT_STEP  5   // 修点步进值（单位：cm）

#define Motor_Speed_GS -400

void INS_Point_Edit(void);       /* [新增] 旋转编码器逐点编辑 */
void INS_FlashWrite_Edit(void);  /* [新增] 将编辑后的INS_data2写回flash */

extern uint8 Encoder_Btn_Short;  /* [新增] 编码器按键短按锁存 */
extern uint8 Encoder_Btn_Long;   /* [新增] 编码器按键长按锁存 */
extern uint8 INS_Flag;
extern uint8 INS_edit_flag;

#endif /* _NAVIGATION_H_ */
