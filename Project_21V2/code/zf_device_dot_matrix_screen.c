#include "zf_common_interrupt.h"
#include "zf_driver_delay.h"
#include "zf_driver_gpio.h"
#include "zf_driver_exti.h"
#include "zf_device_tld7002.h"
#include "zf_device_dot_matrix_screen.h"

/* 7 个行选 GPIO引脚 TLD7002 共 15 路 LED驱动 3 个 5x7 点阵 */
const uint32 dot_matrix_screen_row_pin[] = {
    DOT_MATRIX_SCREEN_SR0_PIN, DOT_MATRIX_SCREEN_SR1_PIN, DOT_MATRIX_SCREEN_SR2_PIN,
    DOT_MATRIX_SCREEN_SR3_PIN, DOT_MATRIX_SCREEN_SR4_PIN, DOT_MATRIX_SCREEN_SR5_PIN,
    DOT_MATRIX_SCREEN_SR6_PIN
};

/* 显示数据缓冲区（3 个字显示，Scan 扫描时 data帧末尾备份给 backup */
int8    dot_matrix_screen_data[3];
int8    dot_matrix_screen_data_backup[3];

/* 亮度全局变量，写入 TLD7002 占空比 */
uint16  dot_matrix_screen_brightness;

/*
 * ASCII 5x7 字体字库，索引 = 字符 - ' '
 * 每行 1 个字节，bit1~bit5 为有效数据位，bit0 不用
 */
const uint8 tld7002_ascii_font_5x7[][7] =
{
    { 0x10, 0x18, 0x1C, 0x1E, 0x1C, 0x18, 0x10},
    { 0x1E, 0x1C, 0x18, 0x10, 0x18, 0x1C, 0x1E},
    { 0x14, 0x14, 0x14, 0x00, 0x00, 0x00, 0x00},
    { 0x14, 0x14, 0x3E, 0x14, 0x3E, 0x14, 0x14},
    { 0x08, 0x3C, 0x0A, 0x1C, 0x28, 0x1E, 0x08},
    { 0x30, 0x32, 0x04, 0x08, 0x10, 0x26, 0x06},
    { 0x0C, 0x12, 0x0A, 0x04, 0x2A, 0x12, 0x2C},
    { 0x0C, 0x08, 0x04, 0x00, 0x00, 0x00, 0x00},
    { 0x10, 0x08, 0x04, 0x04, 0x04, 0x08, 0x10},
    { 0x04, 0x08, 0x10, 0x10, 0x10, 0x08, 0x04},
    { 0x00, 0x08, 0x2A, 0x1C, 0x2A, 0x08, 0x00},
    { 0x00, 0x08, 0x08, 0x3E, 0x08, 0x08, 0x00},
    { 0x00, 0x00, 0x00, 0x00, 0x18, 0x10, 0x08},
    { 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00},
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C},
    { 0x00, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01},
    { 0x1C, 0x22, 0x32, 0x2A, 0x26, 0x22, 0x1C},
    { 0x08, 0x0C, 0x08, 0x08, 0x08, 0x08, 0x1C},
    { 0x1C, 0x22, 0x20, 0x10, 0x08, 0x04, 0x3E},
    { 0x3E, 0x10, 0x08, 0x10, 0x20, 0x22, 0x1C},
    { 0x10, 0x18, 0x14, 0x12, 0x3E, 0x10, 0x10},
    { 0x3E, 0x02, 0x1E, 0x20, 0x20, 0x22, 0x1C},
    { 0x18, 0x04, 0x02, 0x1E, 0x22, 0x22, 0x1C},
    { 0x3E, 0x20, 0x10, 0x08, 0x04, 0x04, 0x04},
    { 0x1C, 0x22, 0x22, 0x1C, 0x22, 0x22, 0x1C},
    { 0x1C, 0x22, 0x22, 0x3C, 0x20, 0x10, 0x0C},
    { 0x00, 0x0C, 0x0C, 0x00, 0x0C, 0x0C, 0x00},
    { 0x00, 0x0C, 0x0C, 0x00, 0x0C, 0x08, 0x04},
    { 0x10, 0x08, 0x04, 0x02, 0x04, 0x08, 0x10},
    { 0x00, 0x00, 0x3E, 0x00, 0x3E, 0x00, 0x00},
    { 0x04, 0x08, 0x10, 0x20, 0x10, 0x08, 0x04},
    { 0x1C, 0x22, 0x20, 0x10, 0x08, 0x00, 0x08},
    { 0x1C, 0x22, 0x20, 0x2C, 0x3A, 0x22, 0x1C},
    { 0x08, 0x14, 0x22, 0x22, 0x3E, 0x22, 0x22},
    { 0x1E, 0x22, 0x22, 0x1E, 0x22, 0x22, 0x1E},
    { 0x1C, 0x22, 0x02, 0x02, 0x02, 0x22, 0x1C},
    { 0x0E, 0x12, 0x22, 0x22, 0x22, 0x12, 0x0E},
    { 0x3E, 0x02, 0x02, 0x1E, 0x02, 0x02, 0x3E},
    { 0x3E, 0x02, 0x02, 0x1E, 0x02, 0x02, 0x02},
    { 0x1C, 0x22, 0x02, 0x3A, 0x22, 0x22, 0x3C},
    { 0x22, 0x22, 0x22, 0x3E, 0x22, 0x22, 0x22},
    { 0x1C, 0x08, 0x08, 0x08, 0x08, 0x08, 0x1C},
    { 0x38, 0x10, 0x10, 0x10, 0x10, 0x12, 0x0C},
    { 0x22, 0x12, 0x0A, 0x06, 0x0A, 0x12, 0x22},
    { 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x3E},
    { 0x22, 0x36, 0x2A, 0x2A, 0x22, 0x22, 0x22},
    { 0x22, 0x22, 0x26, 0x2A, 0x32, 0x22, 0x22},
    { 0x1C, 0x22, 0x22, 0x22, 0x22, 0x22, 0x1C},
    { 0x1E, 0x22, 0x22, 0x1E, 0x02, 0x02, 0x02},
    { 0x1C, 0x22, 0x22, 0x22, 0x2A, 0x12, 0x2C},
    { 0x1E, 0x22, 0x22, 0x1E, 0x0A, 0x12, 0x22},
    { 0x3C, 0x02, 0x02, 0x1C, 0x20, 0x20, 0x1E},
    { 0x3E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08},
    { 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x1C},
    { 0x22, 0x22, 0x22, 0x22, 0x22, 0x14, 0x08},
    { 0x22, 0x22, 0x22, 0x2A, 0x2A, 0x2A, 0x14},
    { 0x22, 0x22, 0x14, 0x08, 0x14, 0x22, 0x22},
    { 0x22, 0x22, 0x22, 0x14, 0x08, 0x08, 0x08},
    { 0x3E, 0x20, 0x10, 0x08, 0x04, 0x02, 0x3E},
    { 0x1C, 0x04, 0x04, 0x04, 0x04, 0x04, 0x1C},
    { 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20},
    { 0x1C, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1C},
    { 0x08, 0x14, 0x22, 0x00, 0x00, 0x00, 0x00},
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3E},
    { 0x04, 0x08, 0x10, 0x00, 0x00, 0x00, 0x00},
    { 0x00, 0x00, 0x1C, 0x20, 0x3C, 0x22, 0x3C},
    { 0x02, 0x02, 0x1A, 0x26, 0x22, 0x22, 0x1E},
    { 0x00, 0x00, 0x1C, 0x02, 0x02, 0x22, 0x1C},
    { 0x20, 0x20, 0x2C, 0x32, 0x22, 0x22, 0x3C},
    { 0x00, 0x00, 0x1C, 0x22, 0x3E, 0x02, 0x1C},
    { 0x18, 0x24, 0x04, 0x0E, 0x04, 0x04, 0x04},
    { 0x00, 0x3C, 0x22, 0x22, 0x3C, 0x20, 0x1C},
    { 0x02, 0x02, 0x1A, 0x26, 0x22, 0x22, 0x22},
    { 0x08, 0x00, 0x0C, 0x08, 0x08, 0x08, 0x1C},
    { 0x10, 0x00, 0x18, 0x10, 0x10, 0x12, 0x0C},
    { 0x02, 0x02, 0x12, 0x0A, 0x06, 0x0A, 0x12},
    { 0x0C, 0x08, 0x08, 0x08, 0x08, 0x08, 0x1C},
    { 0x00, 0x00, 0x16, 0x2A, 0x2A, 0x22, 0x22},
    { 0x00, 0x00, 0x1A, 0x26, 0x22, 0x22, 0x22},
    { 0x00, 0x00, 0x1C, 0x22, 0x22, 0x22, 0x1C},
    { 0x00, 0x1E, 0x22, 0x22, 0x1E, 0x02, 0x02},
    { 0x00, 0x2C, 0x32, 0x32, 0x2C, 0x20, 0x20},
    { 0x00, 0x00, 0x1A, 0x26, 0x02, 0x02, 0x02},
    { 0x00, 0x00, 0x1C, 0x02, 0x1C, 0x20, 0x1E},
    { 0x04, 0x04, 0x0E, 0x04, 0x04, 0x24, 0x18},
    { 0x00, 0x00, 0x22, 0x22, 0x22, 0x32, 0x2C},
    { 0x00, 0x00, 0x22, 0x22, 0x22, 0x14, 0x08},
    { 0x00, 0x00, 0x22, 0x22, 0x2A, 0x2A, 0x14},
    { 0x00, 0x00, 0x22, 0x14, 0x08, 0x14, 0x22},
    { 0x00, 0x22, 0x22, 0x22, 0x3C, 0x20, 0x1C},
    { 0x00, 0x00, 0x3E, 0x10, 0x08, 0x04, 0x3E},
    { 0x0C, 0x04, 0x06, 0x02, 0x06, 0x04, 0x0C},
    { 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08},
    { 0x0C, 0x08, 0x18, 0x10, 0x18, 0x08, 0x0C},
};

/* 箭头图形点阵模组7 行 x 5 列，每行 5 位为有效位，data[] 用 0xFA~0xFF 表示 */
const uint8 triangle_font[7] = {
    0x10,
    0x18,
    0x1C,
    0x1E,
    0x1C,
    0x18,
    0x10,
};

/* 0xFF 双箭头 - 左块（箭头水平旋转） */
const uint8 triangle_font_rotated[7] = {
    0x08,
    0x18,
    0x38,
    0x78,
    0x38,
    0x18,
    0x08,
};

/* 0xFF 双箭头 - 右块（箭头） */
const uint8 triangle_font_ONE[7] = {
    0x00,
    0x00,
    0x08,
    0x08,
    0x08,
    0x00,
    0x00,
};

/* 0xFE 矩形框（外框等） */
const uint8 triangle_font_REXTANGLE[7] = {
    0x00,
    0x00,
    0xFE,
    0xFE,
    0xFE,
    0x00,
    0x00,
};

/* 0xFC 横线（远光灯） */
const uint8 triangle_font_LINE[7] = {
    0x00,
    0xFC,
    0x00,
    0xFC,
    0x00,
    0xFC,
    0x00,
};

/* 0xFB 十字 */
const uint8 triangle_font_CROSS[7] = {
    0x08,
    0xFC,
    0x08,
    0xFC,
    0x08,
    0xFC,
    0x08,
};

/* 0xFD 灯泡 - 左侧灯泡 */
const uint8 triangle_font_DIPP[7] = {
    0x04,
    0x08,
    0x14,
    0x08,
    0x14,
    0x08,
    0x10,
};

/* 0xFD 灯泡 - 中间灯泡 */
const uint8 triangle_font_DIPP_D[7] = {
    0xFE,
    0x02,
    0x02,
    0x02,
    0x02,
    0x02,
    0xFE,
};

/* 0xFD 灯泡 - 右斜侧灯泡 */
const uint8 triangle_font_DIPP_K[7] = {
    0x01,
    0x02,
    0x04,
    0x04,
    0x04,
    0x02,
    0x01,
};
/* 0xFA 雨刷(左) - 左转 */
const uint8 wiper_font_L[7] = {
    0x08,
    0x10,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
};

/* 0xFA 雨刷 - 中间 */
const uint8 wiper_font_M[7] = {
    0x00,
    0x00,
    0x22,
    0x14,
    0x0C,
    0x0C,
    0x0C,
};

/* 0xFA 雨刷 - 右边 */
const uint8 wiper_font_R[7] = {
    0x10,
    0x08,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
};

/* 0xF9 示廓灯 - 左块（← 光束） */
const uint8 position_light_font_L[7] = {
    0x0E,  // ○ • • • ○
    0x00,  // ○ ○ ○ ○ ○
    0x0E,  // ○ • • • ○
    0x00,  // ○ ○ ○ ○ ○
    0x0E,  // ○ • • • ○
    0x00,  // ○ ○ ○ ○ ○
    0x00,  // ○ ○ ○ ○ ○
};

/* 0xF9 示廓灯 - 中块（顶灯） */
const uint8 position_light_font_M[7] = {
    0x0E,  // ○ • • • ○
    0x11,  // • ○ ○ ○ •
    0x11,  // • ○ ○ ○ •
    0x11,  // • • • • •
    0x11,  // • ○ ○ ○ •
    0x11,  // • ○ ○ ○ •
    0x0E,  // ○ • • • ○
};

/* 0xF9 示廓灯 - 右块（→ 光束） */
const uint8 position_light_font_R[7] = {
    0x0E,  // ○ • • • ○
    0x00,  // ○ ○ ○ ○ ○
    0x0E,  // ○ • • • ○
    0x00,  // ○ ○ ○ ○ ○
    0x0E,  // ○ • • • ○
    0x00,  // ○ ○ ○ ○ ○
    0x00,  // ○ ○ ○ ○ ○
};


//-------------------------------------------------------------------------------------------------------------------
//  函数名称      dot_matrix_screen_scan（仅 ASCII 字符显示）
//  函数说明      void
//  输入参数      void
//  使用示例      在 EXTI 同步中断中调用此函数，每隔 dot_matrix_screen_scan1 选一
//  备注          偶数帧时，才调用写入 TLD7002 占空比，奇数帧时更新行选信号前
//-------------------------------------------------------------------------------------------------------------------
void dot_matrix_screen_scan(void)
{
    uint8 i;
    static uint8 entry_num;
    uint8 display_row_now;
    display_row_now = entry_num / 2;
    if(0 == (entry_num % 2))
    {
        if(0 == display_row_now)
        {
            gpio_low(dot_matrix_screen_row_pin[DOT_MATRIX_SCREEN_ROW_NUM - 1]);
        }
        else
        {
            gpio_low(dot_matrix_screen_row_pin[display_row_now - 1]);
        }
        if(7 > display_row_now)
        {
            for(i = 0; 5 > i; i++)
            {
                if((tld7002_ascii_font_5x7[dot_matrix_screen_data_backup[0] - ' '][display_row_now]) & ((uint16)1 << (i + 1)))
                {
                    tld7002_duty[i] = dot_matrix_screen_brightness;
                }
                else
                {
                    tld7002_duty[i] = 0;
                }
            }
            for(; 10 > i; i++)
            {
                if((tld7002_ascii_font_5x7[dot_matrix_screen_data_backup[1] - ' '][display_row_now]) & ((uint16)1 << (i - 5 + 1)))
                {
                    tld7002_duty[i] = dot_matrix_screen_brightness;
                }
                else
                {
                    tld7002_duty[i] = 0;
                }
            }
            for(; 15 > i; i++)
            {
                if((tld7002_ascii_font_5x7[dot_matrix_screen_data_backup[2] - ' '][display_row_now]) & ((uint16)1 << (i - 10 + 1)))
                {
                    tld7002_duty[i] = dot_matrix_screen_brightness;
                }
                else
                {
                    tld7002_duty[i] = 0;
                }
            }
        }
        tld7002_set_duty(1);
    }
    else
    {
        gpio_high(dot_matrix_screen_row_pin[display_row_now]);
        /* TLD7002 占空比写入后立即产生 PWM 输出有效，偶数帧时写入，奇数帧更新行选信号 */
    }
    entry_num++;
    if((DOT_MATRIX_SCREEN_ROW_NUM * 2) <= entry_num)
    {
        entry_num = 0;
        memcpy((uint8 *)dot_matrix_screen_data_backup, (uint8 *)dot_matrix_screen_data, sizeof(dot_matrix_screen_data_backup));
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  函数名称      dot_matrix_screen_scan1（ASCII + 自定义图形版）
//  函数说明      void
//  输入参数      void
//  使用示例      与 dot_matrix_screen_scan 相同，同步中断中调用即可
//  备注          data[n] 为 0xFF~0xFA 时调用自定义图形模式，否则 ASCII 显示
//              0xFF 箭头  0xFE 矩形  0xFD 灯泡  0xFC 线  0xFB 十字  0xFA 雨刷  0xF9 示廓灯
//-------------------------------------------------------------------------------------------------------------------
void dot_matrix_screen_scan1(void)
{
    uint8 i;
    static uint8 entry_num;
    uint8 display_row_now;
    display_row_now = entry_num / 2;
    if(0 == (entry_num % 2))
    {
        if(0 == display_row_now)
        {
            gpio_low(dot_matrix_screen_row_pin[DOT_MATRIX_SCREEN_ROW_NUM - 1]);
        }
        else
        {
            gpio_low(dot_matrix_screen_row_pin[display_row_now - 1]);
        }
        if(7 > display_row_now)
        {
            for(i = 0; 5 > i; i++)
            {
                uint8 char_val = dot_matrix_screen_data_backup[0];
                uint8 pixel_on = 0;
                if(char_val == 0xFF)
                {
                    pixel_on = (triangle_font[display_row_now] >> (i)) & 0x01;
                }
                else if(char_val == 0xFE)
                {
                    pixel_on = (triangle_font_REXTANGLE[display_row_now] >> (i)) & 0x01;
                }
                else if(char_val == 0xFD)
                {
                    pixel_on = (triangle_font_DIPP[display_row_now] >> (i)) & 0x01;
                }
                else if(char_val == 0xFC)
                {
                    pixel_on = (triangle_font_LINE[display_row_now] >> (i)) & 0x01;
                }
                else if(char_val == 0xFB)
                {
                    pixel_on = (triangle_font_CROSS[display_row_now] >> (i)) & 0x01;
                }
                else if(char_val == 0xFA)
                {
                    pixel_on = (wiper_font_L[display_row_now] >> (i)) & 0x01;
                }
                else if(char_val == 0xF9)
                {
                    pixel_on = (position_light_font_L[display_row_now] >> (i)) & 0x01;
                }
                else
                {
                    pixel_on = (tld7002_ascii_font_5x7[char_val - ' '][display_row_now] >> (i + 1)) & 0x01;
                }
                tld7002_duty[i] = pixel_on ? dot_matrix_screen_brightness : 0;
            }
            for(; 10 > i; i++)
            {
                uint8 char_val = dot_matrix_screen_data_backup[1];
                uint8 pixel_on = 0;
                uint8 col_index = i - 4;
                if(char_val == 0xFF)
                {
                    pixel_on = (triangle_font_ONE[display_row_now] >> (col_index)) & 0x01;
                }
                else if(char_val == 0xFE)
                {
                    pixel_on = (triangle_font_REXTANGLE[display_row_now] >> (i - 2)) & 0x01;
                }
                else if(char_val == 0xFD)
                {
                    pixel_on = (triangle_font_DIPP_D[display_row_now] >> (i - 5)) & 0x01;
                }
                else if(char_val == 0xFA)
                {
                    pixel_on = (wiper_font_M[display_row_now] >> (col_index)) & 0x01;
                }
                else if(char_val == 0xF9)
                {
                    pixel_on = (position_light_font_M[display_row_now] >> (i - 5)) & 0x01;
                }
                else
                {
                    pixel_on = (tld7002_ascii_font_5x7[char_val - ' '][display_row_now] >> (col_index + 1)) & 0x01;
                }
                tld7002_duty[i] = pixel_on ? dot_matrix_screen_brightness : 0;
            }
            for(; 15 > i; i++)
            {
                uint8 char_val = dot_matrix_screen_data_backup[2];
                uint8 pixel_on = 0;
                uint8 col_index = i - 7;
                if(char_val == 0xFF)
                {
                    pixel_on = (triangle_font_rotated[display_row_now] >> (col_index)) & 0x01;
                }
                else if(char_val == 0xFE)
                {
                    pixel_on = (triangle_font_REXTANGLE[display_row_now] >> (i - 6)) & 0x01;
                }
                else if(char_val == 0xFD)
                {
                    pixel_on = (triangle_font_DIPP_K[display_row_now] >> (i - 10)) & 0x01;
                }
                else if(char_val == 0xFA)
                {
                    pixel_on = (wiper_font_R[display_row_now] >> (col_index)) & 0x01;
                }
                else if(char_val == 0xF9)
                {
                    pixel_on = (position_light_font_R[display_row_now] >> (i - 10)) & 0x01;
                }
                else
                {
                    pixel_on = (tld7002_ascii_font_5x7[char_val - ' '][display_row_now] >> (col_index + 1)) & 0x01;
                }
                tld7002_duty[i] = pixel_on ? dot_matrix_screen_brightness : 0;
            }
        }
        tld7002_set_duty(1);
    }
    else
    {
        gpio_high(dot_matrix_screen_row_pin[display_row_now]);
    }
    entry_num++;
    if((DOT_MATRIX_SCREEN_ROW_NUM * 2) <= entry_num)
    {
        entry_num = 0;
        memcpy((uint8 *)dot_matrix_screen_data_backup, (uint8 *)dot_matrix_screen_data, sizeof(dot_matrix_screen_data_backup));
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  函数名称      dot_matrix_screen_show_string 显示字符串
//  函数说明      str             待显示字符串，最多 3 个字符，超出部分被原值
//  输入参数      void
//  使用示例      dot_matrix_screen_show_string("OK");
//  备注          写入 data[]后，下一帧扫描时同步备份到 backup[]
//-------------------------------------------------------------------------------------------------------------------
void dot_matrix_screen_show_string(const char *str)
{
    uint8 i;
    for(i = 0; i < 3; i++)
    {
        if(0 == str[i])
        {
            break;
        }
        dot_matrix_screen_data[i] = str[i];
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  函数名称      dot_matrix_screen_set_brightness 设置亮度
//  函数说明      brightness      亮度 0~10000
//  输入参数      void
//  使用示例      dot_matrix_screen_set_brightness(5000);
//-------------------------------------------------------------------------------------------------------------------
void dot_matrix_screen_set_brightness(uint16 brightness)
{
    dot_matrix_screen_brightness = brightness;
}

//-------------------------------------------------------------------------------------------------------------------
//  函数名称      dot_matrix_screen_init 初始化函数
//  函数说明      void
//  输入参数      void
//  使用示例      初始化完成后，需要开启 EXTI 中断调用扫描函数
//-------------------------------------------------------------------------------------------------------------------
void dot_matrix_screen_init(void)
{
    tld7002_init();
    exti_init(DOT_MATRIX_SCREEN_SYNC_PIN, EXTI_TRIGGER_FALLING);
    gpio_init(dot_matrix_screen_row_pin[0], GPO, 0, GPO_PUSH_PULL);
    gpio_init(dot_matrix_screen_row_pin[1], GPO, 0, GPO_PUSH_PULL);
    gpio_init(dot_matrix_screen_row_pin[2], GPO, 0, GPO_PUSH_PULL);
    gpio_init(dot_matrix_screen_row_pin[3], GPO, 0, GPO_PUSH_PULL);
    gpio_init(dot_matrix_screen_row_pin[4], GPO, 0, GPO_PUSH_PULL);
    gpio_init(dot_matrix_screen_row_pin[5], GPO, 0, GPO_PUSH_PULL);
    gpio_init(dot_matrix_screen_row_pin[6], GPO, 0, GPO_PUSH_PULL);
    /* 通过 15 路通道给 TLD7002 同步信号，先将备用通道关闭 */
    tld7002_duty[15] = 5000;
    tld7002_set_duty(1);
    system_delay_ms(10);
    dot_matrix_screen_set_brightness(dot_matrix_screen_brightness);
    dot_matrix_screen_show_string("   ");
}
