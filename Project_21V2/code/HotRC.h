/*
 * HotRC.h
 *
 *  Created on: 2025年6月15日
 *      Author: LiangZhaoqi
 */

#ifndef CODE_HOTRC_H_
#define CODE_HOTRC_H_

/* ========== 遥控通道原始值宏定义（按实际遥控器校准值修改） ========== */
#define CH3_RAW_MIN  336     // 通道3 最小原始值（最大后退）
#define CH3_RAW_MID  1136    // 通道3 中位（摇杆回中）
#define CH3_RAW_MAX  1936    // 通道3 最大原始值（最大前进）

#define CH0_RAW_MIN  48      // 通道0 最小原始值（最大左转）
#define CH0_RAW_MID  848     // 通道0 中位（摇杆回中）
#define CH0_RAW_MAX  1648    // 通道0 最大原始值（最大右转）

void HotRC_control_Init(void);
void HotRC_receive_data_Show(void);
void HotRC_control(void);


#endif /* CODE_HOTRC_H_ */
