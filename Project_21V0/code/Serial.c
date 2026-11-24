/*
 * Serial.c
 *
 *  Created on: 2026年3月23日
 *      Author: liang2281
 */

#include "Serial.h"

uint8_t buffer1[Buffer1_Max]; // 示例缓冲区

int32_t m = 0;
int32_t x = 0;
int32_t y = 0;
int32_t z = 0;

/************************************************
 *  函数名称: 跳过非数字字符（包括正负号）：char* skip_non_numeric_chars_int(const char* str)
 *  功能说明：
 *  参数说明：
 *  参   数：查找的字符串
 *  函数返回：返回第一个数字字符位置
 *  修改时间：2026年3月13日
 *  备   注：注意：已移除对 '.' 的支持，因为现在只解析整数
 ************************************************/
char* skip_non_numeric_chars_int(const char* str)
{
    while(*str != '\0') {
        // 只允许数字、正号、负号
        if ((*str >= '0' && *str <= '9') || *str == '-' || *str == '+') {
            return (char*)str;
        }
        str++;
    }
    return (char*)str;
}

/************************************************
 *  函数名称: 解析串口接收到的"m整数x整数y整数z整数"格式数据：uint8_t Serial_Data_Resolving(int32_t *m, int32_t *x, int32_t *y, int32_t *z)
 *  功能说明：解析串口接收到的"m整数x整数y整数z整数"格式数据
 *  例如: "m1x68y99z88" -> m=1, x=68, y=99, z=88
 *  如果发送 "m1x68.8...", atoi 会截取为 68
 *  参数说明：
 *  参   数：m 输出参数：模式/ID值（整数）
 *  参   数：x 输出参数：X坐标值（整数）
 *  参   数：y 输出参数：Y坐标值（整数）
 *  参   数：z 输出参数：Z坐标值（整数)
 *  函数返回：成功解析标志位
 *  修改时间：2026年3月13日
 *  备   注：
 ************************************************/
uint8_t Serial_Data_Resolving(int32_t *m, int32_t *x, int32_t *y, int32_t *z)
{
    // 调试：打印当前缓冲区内容
    debug_read_ring_buffer(buffer1, 64);

    const char *str_buffer = (const char *)buffer1;

    // 初始化输出参数 (建议取消注释以防万一)
//    *m = 0;
//    *x = 0;
//    *y = 0;
//    *z = 0;

    // 查找 'm' 字符
    char *m_start = strchr(str_buffer, 'm');
    if (m_start == NULL)
    {
        memset(buffer1, 0, sizeof(buffer1));
        return 0;
    }
    m_start++; // 跳过 'm'

    // 查找 'x' 字符 (原代码中你写的是 'p'，这里修正为 'x' 以匹配 "m...x..." 格式)
    // 如果你的协议确实是 'p'，请改回 'p'
    char *x_start = strchr(m_start, 'x');
    if (x_start == NULL)
    {
        memset(buffer1, 0, sizeof(buffer1));
        return 0;
    }

    // --- 解析 m ---
    int32_t m_value = atoi(m_start);
    // 验证 m: 检查起始字符是否合法
    char first_char_m = *skip_non_numeric_chars_int(m_start);
    if ((m_value == 0) && (first_char_m != '0'))
    {
        memset(buffer1, 0, sizeof(buffer1));
        return 0;
    }

    x_start++; // 跳过 'x'

    // 查找 'y' 字符 (原代码中你写的是 'i'，这里修正为 'y')
    char *y_start = strchr(x_start, 'y');
    if (y_start == NULL)
    {
        memset(buffer1, 0, sizeof(buffer1));
        return 0;
    }

    // --- 解析 x ---
    int32_t x_value = atoi(x_start);
    char first_char_x = *skip_non_numeric_chars_int(x_start);
    if ((x_value == 0) && (first_char_x != '0'))
    {
        memset(buffer1, 0, sizeof(buffer1));
        return 0;
    }

    y_start++; // 跳过 'y'

    // 查找 'z' 字符 (原代码中你写的是 'd'，这里修正为 'z')
    char *z_start = strchr(y_start, 'z');
    if (z_start == NULL)
    {
        memset(buffer1, 0, sizeof(buffer1));
        return 0;
    }

    // --- 解析 y ---
    int32_t y_value = atoi(y_start);
    char first_char_y = *skip_non_numeric_chars_int(y_start);
    if ((y_value == 0) && (first_char_y != '0'))
    {
        memset(buffer1, 0, sizeof(buffer1));
        return 0;
    }

    z_start++; // 跳过 'z'

    // --- 解析 z ---
    int32_t z_value = atoi(z_start);
    char first_char_z = *skip_non_numeric_chars_int(z_start);
    if ((z_value == 0) && (first_char_z != '0'))
    {
        memset(buffer1, 0, sizeof(buffer1));
        return 0;
    }

    // 所有验证通过，赋值
    *m = m_value;
    *x = x_value;
    *y = y_value;
    *z = z_value;

    // 成功解析后清空缓冲区
    memset(buffer1, 0, sizeof(buffer1));
    return 1;
}
