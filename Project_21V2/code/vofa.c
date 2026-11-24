/*
 * vofa.c
 *
 *  Created on: 2026年8月11日
 *      Author: LiangZhaoqi
 *      说明：VOFA FireWater 上位机通信模块（移植自 STM32H7 工程，适配 TC387 逐飞库）
 *
 *  移植要点：
 *    - 底层发送由 STM32 的 bsp_debug_uart(DMA) 改为工程库 uart_write_buffer(UART_0, ...)
 *    - 逐飞库 uart_write_buffer 为阻塞发送，因此去掉 STM32 版 DMA 忙标志机制
 *    - 保留 FireWater 协议、传统兼容接口、遥测分组管理、分时发送机制
 *
 *  使用：
 *    vofa_init() 初始化后，周期调用 telemetry_process() 即可自动按分组分时发送；
 *    也可直接调用 vofa_send_tagged_samples / vofa_printf 手动发送。
 */

#include "vofa.h"

#include "IMU.h"            // 遥测数据源：IMU 姿态
#include "Encoder.h"        // 遥测数据源：轮速编码器
#include "Angle_Encoder.h"  // 遥测数据源：转向角度编码器

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* ============================ 协议参数 ============================ */
#define VOFA_FIREWATER_TX_BUF_SIZE      256   /* 单帧发送缓冲区大小 */
#define VOFA_FIREWATER_LEGACY_CH_MAX    16    /* 传统接口最大通道数 */
#define VOFA_FIREWATER_FLOAT_TEXT_SIZE  24    /* 单个浮点文本最大长度 */

/* ===================== 遥测调度参数 ===================== */
#define VOFA_TELEMETRY_SEND_DIV     5   /* 发送分频：每调用 N 次才实际发送一次 */

/* 所有组中最大的通道数（用于采样缓冲区，可随新增分组调大） */
#define VOFA_GROUP_CHANNEL_MAX      16

/* ============================ 串口发送后端（TC387 逐飞库） ============================ */
static bool vofa_uart_transmit(const uint8_t *buf, uint16_t len)
{
    if ((buf == NULL) || (len == 0))
    {
        return false;
    }
    uart_write_buffer(VOFA_UART_INDEX, buf, len);
    return true;
}

/* ============================ FireWater 协议 ============================ */

static float   g_vofa_legacy_samples[VOFA_FIREWATER_LEGACY_CH_MAX];
static uint8_t g_vofa_legacy_channel_count = 0;

static bool vofa_is_tag_valid(const char *tag)
{
    if (tag == NULL)
    {
        return false;
    }
    if (tag[0] == '\0')
    {
        return false;
    }
    if (strcmp(tag, "image") == 0)   /* "image" 为 VOFA+ 图像通道保留标签 */
    {
        return false;
    }
    return true;
}

static bool vofa_send_text_line(char *buf, int32_t len)
{
    if ((buf == NULL) || (len <= 0))
    {
        return false;
    }

    /* 帧尾补充换行符 */
    if (buf[len - 1] != '\n')
    {
        if (len >= (VOFA_FIREWATER_TX_BUF_SIZE - 1))
        {
            return false;
        }
        buf[len]     = '\n';
        buf[len + 1] = '\0';
        len += 1;
    }

    return vofa_uart_transmit((const uint8_t *)buf, (uint16_t)len);
}

void vofa_init(void)
{
    g_vofa_legacy_channel_count = 0;
}

/* ============================ 浮点格式化（避免依赖 %f，兼容 TASKING 库） ============================ */
/**
 * @brief 将浮点数格式化为定点字符串（固定 6 位小数），等价于 sprintf(buf, "%.6f", value)
 *
 * 说明：TASKING 编译器的 snprintf 在读取 %f 的 double 参数时存在问题，
 *       会导致输出异常大数（如 ±2^65）而并非真实数据。
 *       这里改用 %d + 整数运算手动格式化，保证任何嵌入式环境输出都正确。
 *
 * @param buf      输出缓冲区
 * @param buf_size 缓冲区大小（含结尾 '\0'）
 * @param value    待格式化的浮点数
 * @return 写入的字符数（不含结尾 '\0'）
 */
static int32_t vofa_format_float(char *buf, int32_t buf_size, float value)
{
    int32_t ival;
    int32_t frac;
    int32_t sign = 0;
    int32_t len  = 0;

    if ((buf == NULL) || (buf_size <= 0))
    {
        return 0;
    }

    /* 符号处理：取绝对值并标记负号 */
    if (value < 0.0f)
    {
        sign  = 1;
        value = -value;
    }

    /* 整数部分（向零截断） */
    ival = (int32_t)value;

    /* 小数部分：6 位小数并四舍五入 */
    frac = (int32_t)((value - (float)ival) * 1000000.0f + 0.5f);

    /* 小数进位处理：如 0.9999996 -> 1.000000 */
    if (frac >= 1000000)
    {
        ival += 1;
        frac -= 1000000;
    }

    if (sign)
    {
        if (len < (buf_size - 1))
        {
            buf[len++] = '-';
        }
    }

    /* 整数部分 */
    len += snprintf(buf + len, (uint32_t)(buf_size - len), "%d", (int)ival);

    /* 小数点 */
    if (len < (buf_size - 1))
    {
        buf[len++] = '.';
    }

    /* 6 位小数（前导补零） */
    len += snprintf(buf + len, (uint32_t)(buf_size - len), "%06d", (int)frac);

    if (len >= buf_size)
    {
        len = buf_size - 1;
    }
    buf[len] = '\0';

    return len;
}

bool vofa_send_samples(const float *data, uint8_t count)
{
    return vofa_send_tagged_samples(NULL, data, count);
}

bool vofa_send_tagged_samples(const char *tag, const float *data, uint8_t count)
{
    char    tx_buf[VOFA_FIREWATER_TX_BUF_SIZE];
    int32_t write_len = 0;
    uint8_t index     = 0;

    if ((data == NULL) || (count == 0))
    {
        return false;
    }

    tx_buf[0] = '\0';

    if (vofa_is_tag_valid(tag))
    {
        write_len = snprintf(tx_buf, VOFA_FIREWATER_TX_BUF_SIZE, "%s:", tag);
        if ((write_len <= 0) || (write_len >= VOFA_FIREWATER_TX_BUF_SIZE))
        {
            return false;
        }
    }

    for (index = 0; index < count; index++)
    {
        int32_t remain_len = 0;
        int32_t append_len = 0;

        remain_len = VOFA_FIREWATER_TX_BUF_SIZE - write_len;
        if (remain_len <= VOFA_FIREWATER_FLOAT_TEXT_SIZE)
        {
            return false;
        }

        if (index == 0)
        {
            append_len = vofa_format_float(&tx_buf[write_len], remain_len, data[index]);
        }
        else
        {
            if (write_len < (VOFA_FIREWATER_TX_BUF_SIZE - 1))
            {
                tx_buf[write_len] = ',';
            }
            write_len += 1;
            append_len = vofa_format_float(&tx_buf[write_len], remain_len - 1, data[index]);
        }
        if ((append_len <= 0) || (append_len >= remain_len))
        {
            return false;
        }

        write_len += append_len;
    }

    return vofa_send_text_line(tx_buf, write_len);
}

bool vofa_printf(const char *fmt, ...)
{
    char    tx_buf[VOFA_FIREWATER_TX_BUF_SIZE];
    va_list args;
    int32_t write_len = 0;

    if (fmt == NULL)
    {
        return false;
    }

    va_start(args, fmt);
    write_len = vsnprintf(tx_buf, VOFA_FIREWATER_TX_BUF_SIZE, fmt, args);
    va_end(args);

    if ((write_len <= 0) || (write_len >= VOFA_FIREWATER_TX_BUF_SIZE))
    {
        return false;
    }

    return vofa_send_text_line(tx_buf, write_len);
}

/* ============================ 传统兼容接口 ============================ */

void vofa_start(void)
{
    vofa_init();
}

void vofa_send_data(uint8_t num, float data)
{
    if (num >= VOFA_FIREWATER_LEGACY_CH_MAX)
    {
        return;
    }

    g_vofa_legacy_samples[num] = data;
    if (num >= g_vofa_legacy_channel_count)
    {
        g_vofa_legacy_channel_count = (uint8_t)(num + 1);
    }
}

void vofa_sendframetail(void)
{
    if (g_vofa_legacy_channel_count == 0)
    {
        return;
    }

    vofa_send_samples(g_vofa_legacy_samples, g_vofa_legacy_channel_count);
    g_vofa_legacy_channel_count = 0;
}

void vofa_demo(void)
{
    static float demo_t = 0.0f;
    float        demo_data[4];

    demo_t += 0.1f;
    demo_data[0] = demo_t;
    demo_data[1] = demo_t * 0.5f;
    demo_data[2] = demo_t * -0.25f;
    demo_data[3] = demo_t * -0.5f;

    vofa_send_tagged_samples("demo", demo_data, 4);
}

/* ============================ 链路测试（验证串口/VOFA 用） ============================ */
void vofa_test(void)
{
    static uint32_t test_cnt = 0;
    float           test_data[4];

    test_cnt++;

    /* 1) 文本测试：在 VOFA+ 左下角日志或串口助手里可见 */
    vofa_printf("VOFA FireWater test ok, cnt=%d", (int)test_cnt);

    /* 2) 数值测试：4 通道带标签帧，波形便于在 VOFA+ 上观察 */
    test_data[0] = (float)(test_cnt % 200);          /* 锯齿波 0~199 */
    test_data[1] = (float)(test_cnt % 100) - 50.0f;  /* 三角波 -50~49 */
    test_data[2] = (float)test_cnt * 0.5f;
    test_data[3] = (float)test_cnt * -0.25f;

    vofa_send_tagged_samples("test", test_data, 4);
}

/* ============================ 遥测分组管理 + 分时发送 ============================ */

/* 调度器内部状态 */
static uint8_t g_telemetry_send_div_cnt = 5;   /* 分频计数器 */
static uint8_t g_telemetry_group_phase  = 0;   /* 当前轮询到的组 */

/* 采样函数前向声明 */
static void telemetry_sample_demo(float *channels);
static void telemetry_sample_imu(float *channels);
static void telemetry_sample_encoder(float *channels);
static void telemetry_sample_angle_encoder(float *channels);
static void telemetry_sample_hotrc_receive_data(float *channels);
static void telemetry_sample_motor_pid_data(float *channels);

/* 遥测组配置表：{ 标签, 通道数, 使能, 采样函数 }
 * 标签:    VOFA+ 通道组名，用于区分不同数据源
 * 使能:    1=启用 0=禁用（跳过该组，可在运行时用 telemetry_set_enable 修改）
 * 采样函数: 填充 channels 缓冲区的回调，返回该组的通道数据 */
static vofa_telemetry_group_t g_telemetry_groups[VOFA_GROUP_COUNT] = {
    {"Demo", 4, 0, telemetry_sample_demo},                   /* 演示波形：验证链路 */
    {"IMU", 4, 0, telemetry_sample_imu},                     /* IMU: Roll/Pitch/Yaw/yaw */
    {"Encoder", 4, 0, telemetry_sample_encoder},             /* 编码器: 后轮左右轮速 */
    {"Angle_Encoder", 3, 1, telemetry_sample_angle_encoder}, /* 转向角度编码器: 实际/实际转换/目标 */
    {"HotRC", 7, 0, telemetry_sample_hotrc_receive_data},    /* 遥控器值 */
    {"Motor", 14, 0, telemetry_sample_motor_pid_data},       /* 左右电机数据 */
};

static void telemetry_sample_demo(float *channels)
{
    static float demo_t = 0.0f;

    demo_t += 0.1f;
    channels[0] = demo_t;
    channels[1] = demo_t * 0.5f;
    channels[2] = demo_t * -0.25f;
    channels[3] = demo_t * -0.5f;
}

static void telemetry_sample_imu(float *channels)
{
    channels[0] = (float)Roll;  //IMU角度
    channels[1] = (float)Pitch;
    channels[2] = (float)Yaw;
    channels[3] = (float)yaw;
}

static void telemetry_sample_encoder(float *channels)
{
    channels[0] = (float)Unit_Time_Read_Encoder_Value(TIM2_ENCODER);  /* 右轮 */
    channels[1] = (float)Unit_Time_Read_Encoder_Value(TIM5_ENCODER);  /* 左轮 */
    channels[2] = (float)Read_Encoder_Aggregate_Value(TIM2_ENCODER);  /* 右轮 */
    channels[3] = (float)Read_Encoder_Aggregate_Value(TIM5_ENCODER);  /* 左轮 */
}

static void telemetry_sample_angle_encoder(float *channels)
{
    channels[0] = (float)absolute_encoder_get_location();
    channels[1] = (float)Angle_Encoder_Get_Value();   /* 转向实际角度 */
    channels[2] = (float)Angle_Target_Position;       /* 转向目标角度 */
}

static void telemetry_sample_hotrc_receive_data(float *channels)
{
    channels[0] = (float)uart_receiver.state;
    channels[1] = (float)uart_receiver.channel[0];
    channels[2] = (float)uart_receiver.channel[1];
    channels[3] = (float)uart_receiver.channel[2];
    channels[4] = (float)uart_receiver.channel[3];
    channels[5] = (float)uart_receiver.channel[4];
    channels[6] = (float)uart_receiver.channel[5];
}

static void telemetry_sample_motor_pid_data(float *channels)
{
    Vofa_Back_Motor_PID_Data(channels,14);
}

    /* 查找下一个已使能的遥测组（轮询不越界，全禁用时返回自身） */
static uint8_t telemetry_get_next_enabled_group(uint8_t group)
{
    uint8_t next_group = group;
    uint8_t scan_cnt   = 0;

    for (scan_cnt = 0; scan_cnt < VOFA_GROUP_COUNT; scan_cnt++)
    {
        next_group = (uint8_t)((next_group + 1) % VOFA_GROUP_COUNT);
        if (g_telemetry_groups[next_group].enable)
        {
            return next_group;
        }
    }

    return group;   /* 所有组都未启用，返回自身 */
}

void telemetry_init(void)
{
    g_telemetry_send_div_cnt = 0;
    /* 从最后一个组位置开始，找到第一个 enable 的组作为起始 */
    g_telemetry_group_phase =
        telemetry_get_next_enabled_group((uint8_t)(VOFA_GROUP_COUNT - 1));
}

void telemetry_set_enable(uint8_t group, uint8_t enable)
{
    if (group >= VOFA_GROUP_COUNT)
    {
        return;
    }
    g_telemetry_groups[group].enable = enable;
}

uint8_t telemetry_get_group_count(void)
{
    return (uint8_t)VOFA_GROUP_COUNT;
}

void telemetry_process(void)
{
    float                   channels[VOFA_GROUP_CHANNEL_MAX];
    vofa_telemetry_group_t *group_cfg = NULL;

    /* 分频控制：未到发送周期则直接返回 */
    g_telemetry_send_div_cnt++;
    if (g_telemetry_send_div_cnt < VOFA_TELEMETRY_SEND_DIV)
    {
        return;
    }
    g_telemetry_send_div_cnt = 0;

    /* 获取当前组配置 */
    group_cfg = &g_telemetry_groups[g_telemetry_group_phase];

    /* 当前组无效则跳到下一组 */
    if ((!group_cfg->enable) || (group_cfg->sample_func == NULL))
    {
        g_telemetry_group_phase =
            telemetry_get_next_enabled_group(g_telemetry_group_phase);
        return;
    }

    /* 采样并发送 */
    memset(channels, 0, sizeof(channels));
    group_cfg->sample_func(channels);
    if (vofa_send_tagged_samples(group_cfg->tag, channels, group_cfg->channel_count))
    {
        /* 发送成功，切换到下一组（轮询调度） */
        g_telemetry_group_phase =
            telemetry_get_next_enabled_group(g_telemetry_group_phase);
    }
}
