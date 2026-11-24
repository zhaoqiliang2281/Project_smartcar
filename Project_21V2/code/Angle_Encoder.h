/*
 * Angle_Encoder.h
 *
 *  Created on: 2026年4月22日
 *      Author: LiangZhaoqi
 */
#ifndef CODE_ANGLE_ENCODER_H_
#define CODE_ANGLE_ENCODER_H_

#include "zf_common_headfile.h"

#define Dir_Mtor_USE_PID_ENCODER    (0)
#define Dir_Mtor_USE_PID_STRUCT     (0)

#if Dir_Mtor_USE_PID_ENCODER

//前轮转向电机目标值
extern int32 Target_Position;

void Dir_Motor_Encoder_PID_Update(void);

#else
#endif

#define MAXIMUM_LEFT_AND_RIGHT_STEERING_ANGLE  (937)

#define  RIGHT_ANGLE_ENCODER_LIMIT_MAX      (3448)
#define   LEFT_ANGLE_ENCODER_LIMIT_MAX      (1325)
#define        ANGLE_ENCODER_MID_VALUE      (2415)


#define ANGLE_ENCODER_PID_KP    6.68f    // 比例系数
#define ANGLE_ENCODER_PID_KI     0.1f    // 积分系数
#define ANGLE_ENCODER_PID_KD    0.01f    // 微分系数

extern int32 Angle_Target_Position;

void Angle_Encoder_Init(void);
int16 Angle_Encoder_Get_Value(void);
void Angle_Encoder_PID_Update(void);

void Angle_Encoder_Test_Show(void);

#endif /* CODE_ANGLE_ENCODER_H_ */
