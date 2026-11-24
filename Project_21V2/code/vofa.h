/*
 * vofa.h
 *
 *  Created on: 2026年8月11日
 *      Author: LiangZhaoqi
 *      说明：VOFA FireWater 上位机通信模块（移植自 STM32H7 工程，适配 TC387 逐飞库）
 *
 *  功能：
 *    1. VOFA FireWater 协议实现（带标签发送 / 无标签发送 / 文本调试）
 *    2. 调试串口通信（使用工程库 uart_write_buffer 发送，串口 UART_0）
 *    3. 支持带标签的数据发送（VOFA+ 多波形通道）
 *    4. 传统兼容接口（vofa_start / vofa_send_data / vofa_sendframetail）
 *    5. 遥测数据分组管理（telemetry_set_enable 动态使能/禁用分组）
 *    6. 多个数据源的分时发送机制（telemetry_process 轮询调度）
 */

#ifndef __VOFA_H
#define __VOFA_H

#include "zf_common_headfile.h"  // 包含逐飞科技公共头文件

/* ============================ 移植配置 ============================ */
#define VOFA_UART_INDEX   (UART_0)   // VOFA 数据发送使用的串口（串口0，由 debug_init 初始化）

/* ======================== FireWater 协议接口 ======================== */
void   vofa_init(void);                                            // 初始化 VOFA 模块
bool   vofa_send_samples(const float *data, uint8_t count);                    // 发送一组浮点数据（无标签）
bool   vofa_send_tagged_samples(const char *tag, const float *data, uint8_t count); // 发送带标签的数据（VOFA+ 多通道）
bool   vofa_printf(const char *fmt, ...);                           // 发送文本（调试用）

/* ======================== 传统兼容接口 ======================== */
void   vofa_start(void);                         // 兼容旧接口：初始化
void   vofa_send_data(uint8_t num, float data);  // 兼容旧接口：缓存一个通道数据
void   vofa_sendframetail(void);                 // 兼容旧接口：将缓存的数据作为一帧发送
void   vofa_demo(void);                          // 演示：发送一组 demo 波形

/* ======================== 链路测试 ======================== */
void   vofa_test(void);                          // 测试：发送文本横幅 + 一组数值帧，验证串口链路

/* ======================== 遥测分组管理 ======================== */
typedef void (*vofa_sample_func_t)(float *channels);   // 采样回调：填充 float 通道数组

typedef struct {
    const char         *tag;           /* VOFA+ 标签名 */
    uint8_t             channel_count; /* 通道数 */
    uint8_t             enable;        /* 使能标志：1=启用 0=禁用 */
    vofa_sample_func_t  sample_func;   /* 采样回调 */
} vofa_telemetry_group_t;

/* 遥测组索引（顺序须与 vofa.c 内组配置表一致） */
typedef enum
{
    VOFA_GROUP_DEMO = 0,      /* 演示波形组 */
    VOFA_GROUP_IMU,           /* IMU 姿态组 */
    VOFA_GROUP_ENCODER,       /* 编码器组 */
    VOFA_GROUP_ANGLE_ENCODER, /* 转向角度编码器组 */
    VOFA_GROUP_HOTRC,         /* 遥控器组 */
    VOFA_GROUP_MOTOR_PID,     /* 后轮电机PID组 */

    VOFA_GROUP_COUNT /* 组总数 */
} vofa_group_e;

void    telemetry_init(void);                    // 初始化遥测调度器
void    telemetry_process(void);                 // 遥测调度处理（周期调用，分时发送）
void    telemetry_set_enable(uint8_t group, uint8_t enable);  // 使能/禁用指定分组
uint8_t telemetry_get_group_count(void);         // 获取分组总数

#endif /* __VOFA_H */
