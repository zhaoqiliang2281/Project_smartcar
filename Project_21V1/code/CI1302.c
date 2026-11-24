#include "zf_common_headfile.h"


#pragma section all "cpu0_dsram"


static uint8 uart_get_data[64];         // 定义UART接收数据缓冲区，用于存储原始串口数据
static uint8 fifo_get_data[64];         // 定义FIFO读取数据缓冲区
static uint8 get_data = 0;              // 定义一个字节变量用于暂存接收到的数据
static uint32 fifo_data_count = 0;      // FIFO中当前数据字节数
static fifo_struct uart_data_fifo;      // 定义UART数据FIFO结构体变量
static uint8 ci1302_uart_index = 0;     // 存储当前使用的串口索引

static const uint8 frame_head[FRAME_HEAD_LEN] = {0x6B, 0x79, 0x00, 0x81};       // 定义帧头数据（固定帧头：6B 79 00 81）
static uint8 frame_buffer[FRAME_TOTAL_LEN];                                     // 帧数据缓存区，存放完整一帧数据
static uint8 frame_index = 0;                                                   // 帧数据当前接收位置索引
uint8 data_store[DATA_STORE_MAX];                                               // 存储已接收命令码的数组
uint32 data_store_count = 0;                                                    // 已存储命令码的数量

uint8 Sport_sum_flag=0;     //运动任务数量合计
uint8 Flash_sum_flag=0;     //灯光任务数量合计
uint8 Voice_sum_flag=0;     //鸣笛任务数量合计

uint8 Chinese_task_display=0;

/**
 * @brief 根据命令码获取对应的命令名称字符串
 * @param command 命令码
 * @return 命令名称字符串指针
 */
const char *CI1302_get_command_name(uint8 command)
{
    switch(command)
    {
        case 0x01:
            return "你好科宇智能管家";
        case 0x02:
            return "欢迎语";
        case 0x03:
            return "休息语";
        case 0x04:
            Chinese_task_display++;
            ips200_show_chinese(0, 16*Chinese_task_display,   16, (const uint8 *)left_light, 6, RGB565_WHITE);
            return "打开左转向灯";
        case 0x05:
            Chinese_task_display++;
            ips200_show_chinese(0, 16*Chinese_task_display,   16, (const uint8 *)right_light, 6, RGB565_WHITE);
            return "打开右转向灯";
        case 0x06:
            Chinese_task_display++;
            ips200_show_chinese(0, 16*Chinese_task_display,   16, (const uint8 *)far_light, 5, RGB565_WHITE);
            return "打开远光灯";
        case 0x07:
            Chinese_task_display++;
            ips200_show_chinese(0, 16*Chinese_task_display,   16, (const uint8 *)near_light, 5, RGB565_WHITE);
            return "打开近光灯";
        case 0x08:
            Chinese_task_display++;
            ips200_show_chinese(0, 16*Chinese_task_display,   16, (const uint8 *)Fog_lights, 4, RGB565_WHITE);
            return "打开雾灯";
        case 0x09:
            Chinese_task_display++;
            ips200_show_chinese(0, 16*Chinese_task_display,   16, (const uint8 *)emergency_light, 5, RGB565_WHITE);
            return "打开双闪灯";
        case 0x0A:
            Chinese_task_display++;
            ips200_show_chinese(0, 16*Chinese_task_display,   16, (const uint8 *)Interior_lighting_fixtures, 7, RGB565_WHITE);
            return "打开车内照明灯";
        case 0x0B:
            Chinese_task_display++;
            ips200_show_chinese(0, 16*Chinese_task_display,   16, (const uint8 *)wiper, 5, RGB565_WHITE);
            return "打开雨刷器";
        case 0x0C:
            Chinese_task_display++;
            ips200_show_chinese(0, 16*Chinese_task_display,   16, (const uint8 *)Horn_for_one_second, 5, RGB565_WHITE);
            return "鸣笛一秒钟";
        case 0x0D:
            Chinese_task_display++;
            ips200_show_chinese(0, 16*Chinese_task_display,   16, (const uint8 *)Sound_the_horn_for_two_seconds, 5, RGB565_WHITE);
            return "鸣笛两秒钟";
        case 0x0E:
            Chinese_task_display++;
            ips200_show_chinese(0, 16*Chinese_task_display,   16, (const uint8 *)Sound_the_horn_for_three_seconds, 5, RGB565_WHITE);
            return "鸣笛三秒钟";
        case 0x0F:
            Chinese_task_display++;
            ips200_show_chinese(0, 16*Chinese_task_display,   16, (const uint8 *)Honk_the_horn_twice, 4, RGB565_WHITE);
            return "鸣笛两声";
        case 0x10:
            Chinese_task_display++;
            ips200_show_chinese(0, 16*Chinese_task_display,   16, (const uint8 *)HSound_the_horn_three, 4, RGB565_WHITE);
            return "鸣笛三声";
        case 0x11:
            Chinese_task_display++;
            ips200_show_chinese(0, 16*Chinese_task_display,   16, (const uint8 *)HSound_the_horn_four, 4, RGB565_WHITE);
            return "鸣笛四声";
        case 0x12:
            Chinese_task_display++;
            ips200_show_chinese(0, 16*Chinese_task_display,   16, (const uint8 *)Long_and_shortly_blasts_horn, 4, RGB565_WHITE);
            return "长短鸣笛";
        case 0x13:
            Chinese_task_display++;
            ips200_show_chinese(0, 16*Chinese_task_display,   16, (const uint8 *)Sharp_whistle_blasts_horn, 4, RGB565_WHITE);
            return "急促鸣笛";
        case 0x14:
            Chinese_task_display++;
            ips200_show_chinese(0, 16*Chinese_task_display,   16, (const uint8 *)Alarm_siren_blasts_horn, 4, RGB565_WHITE);
            return "警报鸣笛";
        case 0x15:
            Chinese_task_display++;
            ips200_show_chinese(0, 16*Chinese_task_display,   16, (const uint8 *)the_left_side_the_onedoorway, 7, RGB565_WHITE);
            return "通过门洞一左侧";
        case 0x16:
            Chinese_task_display++;
            ips200_show_chinese(0, 16*Chinese_task_display,   16, (const uint8 *)the_side_the_onedoorway, 5, RGB565_WHITE);
            return "通过门洞一";
        case 0x17:
            Chinese_task_display++;
            ips200_show_chinese(0, 16*Chinese_task_display,   16, (const uint8 *)the_side_the_twodoorway, 5, RGB565_WHITE);
            return "通过门洞二";
        case 0x18:
            Chinese_task_display++;
            ips200_show_chinese(0, 16*Chinese_task_display,   16, (const uint8 *)the_side_the_threedoorway, 5, RGB565_WHITE);
            return "通过门洞三";
        case 0x19:
            Chinese_task_display++;
            ips200_show_chinese(0, 16*Chinese_task_display,   16, (const uint8 *)the_right_side_the_threedoorway, 7, RGB565_WHITE);
            return "通过门洞三右侧";
        case 0x1A:
            Chinese_task_display++;
            ips200_show_chinese(0, 16*Chinese_task_display,   16, (const uint8 *)On_the_right_side_the_onedoorway, 7, RGB565_WHITE);
            return "门洞一右侧返回";
        case 0x1B:
            Chinese_task_display++;
            ips200_show_chinese(0, 16*Chinese_task_display,   16, (const uint8 *)On_side_the_onedoorway, 5, RGB565_WHITE);
            return "门洞一返回";
        case 0x1C:
            Chinese_task_display++;
            ips200_show_chinese(0, 16*Chinese_task_display,   16, (const uint8 *)On_side_the_twodoorway, 5, RGB565_WHITE);
            return "门洞二返回";
        case 0x1D:
            Chinese_task_display++;
            ips200_show_chinese(0, 16*Chinese_task_display,   16, (const uint8 *)On_side_the_threedoorway, 5, RGB565_WHITE);
            return "门洞三返回";
        case 0x1E:
            Chinese_task_display++;
            ips200_show_chinese(0, 16*Chinese_task_display,   16, (const uint8 *)On_the_Lt_side_the_threedoorway, 7, RGB565_WHITE);
            return "门洞三左侧返回";
        case 0x1F:
            Chinese_task_display++;
            ips200_show_chinese(0, 16*Chinese_task_display,   16, (const uint8 *)Move_forward_ten_meters, 4, RGB565_WHITE);
            return "前行十米";
        case 0x20:
            Chinese_task_display++;
            ips200_show_chinese(0, 16*Chinese_task_display,   16, (const uint8 *)Move_back_ten_meters, 4, RGB565_WHITE);
            return "后退十米";
        case 0x21:
            Chinese_task_display++;
            ips200_show_chinese(0, 16*Chinese_task_display,   16, (const uint8 *)Move_snakelike_pattern_tenmeters, 6, RGB565_WHITE);
            return "蛇形前进十米";
        case 0x22:
            Chinese_task_display++;
            ips200_show_chinese(0, 16*Chinese_task_display,   16, (const uint8 *)Back_snakelike_pattern_tenmeters, 6, RGB565_WHITE);
            return "蛇形后退十米";
        case 0x23:
            Chinese_task_display++;
            ips200_show_chinese(0, 16*Chinese_task_display,   16, (const uint8 *)Turn_counterclockwise_onecircle, 6, RGB565_WHITE);
            return "逆时针转一圈";
        case 0x24:
            Chinese_task_display++;
            ips200_show_chinese(0, 16*Chinese_task_display,   16, (const uint8 *)Turn_clockwise_onecircle, 6, RGB565_WHITE);
            return "顺时针转一圈";
        case 0x25:
            Chinese_task_display++;
            ips200_show_chinese(0, 16*Chinese_task_display,   16, (const uint8 *)Turn_Left, 2, RGB565_WHITE);
            return "左转";
        case 0x26:
            Chinese_task_display++;
            ips200_show_chinese(0, 16*Chinese_task_display,   16, (const uint8 *)Turn_Right, 2, RGB565_WHITE);
            return "右转";
        default:
            return "未知命令";
    }
}
/**
 * @brief 执行具体的硬件控制命令
 * @param command 命令码
 */
void execute_command(uint8 command)
{
    switch(command)
    {
        case 0x04:
            left_turn_signal();//左转
            break;//return "打开左转向灯";
        case 0x05:
            Right_turn_signal();
            break;//return "打开右转向灯";
        case 0x06:
            headlights_on_full_beam();//远光灯
            break;//return "打开远光灯";
        case 0x07:
            dipped_headlight(); //近光灯
            break;//return "打开近光灯";
        case 0x08:
            foglight();
            break;//return "打开雾灯";
        case 0x09:
            emergency_flasher();//双闪灯
            break;//"打开双闪灯";
        case 0x0A:
            position_light();
            break;// "打开示廓灯";
        case 0x0B:
            windshield_wiper_on();
            break;// "打开雨刷器";
        case 0x0C:
            BY9301_Say(1);
            BY9301_setVolume(25);
            system_delay_ms(2000);
            break;//return "鸣笛一秒钟";
        case 0x0D:
            BY9301_Say(2);
            BY9301_setVolume(25);
            system_delay_ms(3000);
            break;//return "鸣笛两秒钟";
        case 0x0E:
            BY9301_Say(3);
            BY9301_setVolume(25);
            system_delay_ms(4000);
            break;//return "鸣笛三秒钟";
        case 0x0F:
            BY9301_Say(4);
            BY9301_setVolume(25);
            system_delay_ms(4000);
            break;//return "鸣笛两声";
        case 0x10:
            BY9301_Say(5);
            BY9301_setVolume(25);
            system_delay_ms(6000);
            break;//return "鸣笛三声";
        case 0x11:
            BY9301_Say(6);
            BY9301_setVolume(25);
            system_delay_ms(8000);
            break;//return "鸣笛四声";
        case 0x12:
            BY9301_Say(7);
            BY9301_setVolume(25);
            system_delay_ms(6000);
            break;//return "长短鸣笛";
        case 0x13:
            BY9301_Say(8);
            BY9301_setVolume(25);
            system_delay_ms(3000);
            break;//急促鸣笛
        case 0x14:
            BY9301_Say(9);
            BY9301_setVolume(25);
            system_delay_ms(7000);
            break;
        case 0x15:
            break;
        case 0x16:
            break;
        case 0x17:
            break;
        case 0x18:
            break;
        case 0x19:
            break;
        case 0x1A:
            break;
        case 0x1B:
            break;
        case 0x1C:
            break;
        case 0x1D:
            break;
        case 0x1E:
            break;
        case 0x1F:
            break;
        case 0x20:
            break;
        case 0x21:
            break;
        case 0x22:
            break;
        case 0x23:
            break;
        case 0x24:
            break;
        case 0x25:
            break;
        case 0x26:
            break;
        default:
            break;
    }
}
/**
 * @brief 将16进制字符转换成半字节数值
 * @param ch ASCII字符（0-9, A-F, a-f）
 * @return 对应的半字节值，无效则返回0xFF
 */
static uint8 uart_hex_char_to_nibble(uint8 ch)
{
    if((ch >= '0') && (ch <= '9'))
    {
        return (uint8)(ch - '0');
    }

    if((ch >= 'A') && (ch <= 'F'))
    {
        return (uint8)(ch - 'A' + 10);
    }

    if((ch >= 'a') && (ch <= 'f'))
    {
        return (uint8)(ch - 'a' + 10);
    }

    return 0xFF;
}
/**
 * @brief 判断缓冲区数据是否全部为合法的ASCII十六进制字符（忽略空格、回车等）
 * @param buffer 数据缓冲区
 * @param length 数据长度
 * @return 1-是合法十六进制流，0-不是
 */
static uint8 uart_is_ascii_hex_stream(uint8 *buffer, uint32 length)
{
    uint32 i = 0;

    for(i = 0; i < length; i++)
    {
        if((buffer[i] == ' ') || (buffer[i] == '\r') || (buffer[i] == '\n') || (buffer[i] == '\t'))
        {
            continue;
        }

        if(uart_hex_char_to_nibble(buffer[i]) == 0xFF)
        {
            return 0;
        }
    }

    return 1;
}
/**
 * @brief 将ASCII十六进制字符串解码为原始字节数据
 * @param buffer 输入ASCII十六进制缓冲区
 * @param length 输入数据长度
 * @param decoded 输出解码后的字节数组
 * @param decoded_length 输出解码后的字节长度
 * @return 1-解码成功，0-解码失败（奇数个十六进制字符）
 */
static uint8 uart_decode_ascii_hex_stream(uint8 *buffer, uint32 length, uint8 *decoded, uint32 *decoded_length)
{
    uint32 i = 0;
    uint8 high_nibble = 0xFF;
    uint8 nibble = 0;

    *decoded_length = 0;

    for(i = 0; i < length; i++)
    {
        if((buffer[i] == ' ') || (buffer[i] == '\r') || (buffer[i] == '\n') || (buffer[i] == '\t'))
        {
            continue;
        }

        nibble = uart_hex_char_to_nibble(buffer[i]);
        if(nibble == 0xFF)
        {
            return 0;
        }

        if(high_nibble == 0xFF)
        {
            high_nibble = nibble;
        }
        else
        {
            decoded[(*decoded_length)++] = (uint8)((high_nibble << 4) | nibble);
            high_nibble = 0xFF;
        }
    }

    if(high_nibble != 0xFF)
    {
        return 0;
    }

    return 1;
}

/**
 * @brief 通过串口以十六进制格式输出一个字节（例如输出"0A"）
 * @param value 要输出的字节值
 */
static void uart_print_hex_byte(uint8 value)
{
    const char hex_table[] = "0123456789ABCDEF";

    uart_write_byte(ci1302_uart_index, hex_table[(value >> 4) & 0x0F]);
    uart_write_byte(ci1302_uart_index, hex_table[value & 0x0F]);
}

/**
 * @brief 输出data_store中存储的所有命令码（十六进制）
 */
static void uart_print_data_store(void)
{
    uint32 i = 0;

    for(i = 0; i < data_store_count; i++)
    {
        uart_write_byte(ci1302_uart_index, ' ');
        uart_print_hex_byte(data_store[i]);
    }
}

/**
 * @brief 处理一帧完整的命令数据
 * @param frame 指向完整帧数据的指针
 */
static void uart_frame_process(uint8 *frame)
{
    uint8 command = frame[4];
    const char *cmd_name;
 if(command != 0x01 && command != 0x02 && command != 0x03 ){

    if(data_store_count < DATA_STORE_MAX)
    {
        if(command >= 0x04 && command <= 0x0B)          //收到灯光任务让标志位+1
        {
            Flash_sum_flag++;
        }
        else if(command >= 0x0C && command <= 0x14)     //收到鸣笛任务让标志位+1
        {
            Voice_sum_flag++;
        }
        else if(command >= 0x1F && command <= 0x26)     //收到动作任务让标志位+1
        {
            Sport_sum_flag++;
        }

        if(command==0x27)
        {
            command=0x28;
            ips200_show_string(120, 16*17, "GO_KM2");   //发车口令“开始发车”
            Buzzer_SetTimer(50);
            Km2_protect_flag = 1;
            dispatch_car = 1;
        }
        data_store[data_store_count++] = command;

        /* 打印命令码 */
        uart_print_hex_byte(command);

        /* 打印命令名称 */
        cmd_name = CI1302_get_command_name(command);
        uart_write_string(ci1302_uart_index, " ");
        uart_write_string(ci1302_uart_index, cmd_name);

        /* 打印所有已存储的命令 */
        uart_print_data_store();

        /* 添加换行 */
        uart_write_byte(ci1302_uart_index, '\r');
        uart_write_byte(ci1302_uart_index, '\n');

        /* 只存储不执行, 由KM2_StateMachine统一调度 */
    }
  }
    else
    {
        uart_write_string(ci1302_uart_index, "\r\nDATA STORE FULL");
    }
}

/**
 * @brief 处理接收到的原始数据流，基于帧头帧尾进行帧同步
 * @param buffer 数据缓冲区
 * @param length 数据长度
 */
static void uart_process_stream(uint8 *buffer, uint32 length)
{
    uint32 i = 0;

    for(i = 0; i < length; i++)
    {
        uint8 data = buffer[i];

        if(frame_index == 0)
        {
            if(data == frame_head[0])
            {
                frame_buffer[frame_index++] = data;
            }
            continue;
        }

        if(frame_index < FRAME_HEAD_LEN)
        {
            if(data == frame_head[frame_index])
            {
                frame_buffer[frame_index++] = data;
            }
            else
            {
                frame_index = (data == frame_head[0]) ? 1 : 0;
                if(frame_index == 1)
                {
                    frame_buffer[0] = data;
                }
            }
            continue;
        }

        frame_buffer[frame_index++] = data;

        if(frame_index == FRAME_TOTAL_LEN)
        {
            if(frame_buffer[FRAME_TOTAL_LEN - 1] == FRAME_TAIL)
            {
                uart_frame_process(frame_buffer);
            }
            frame_index = 0;
        }
    }
}

/**
 * @brief 解析FIFO中的串行数据，支持ASCII十六进制流和原始二进制流
 */
static void uart_stream_parser(void)
{
    uint8 decoded_buffer[64];
    uint32 decoded_length = 0;

    if(uart_is_ascii_hex_stream(fifo_get_data, fifo_data_count))
    {
        if(uart_decode_ascii_hex_stream(fifo_get_data, fifo_data_count, decoded_buffer, &decoded_length))
        {
            uart_process_stream(decoded_buffer, decoded_length);
        }
        return;
    }

    uart_process_stream(fifo_get_data, fifo_data_count);
}

/**
 * @brief 初始化CI1302语音模块的串口通信
 * @param uart_index 串口索引（例如UART0, UART1等）
 * @param baudrate 波特率
 * @param uart_tx_pin TX引脚配置
 * @param uart_rx_pin RX引脚配置
 */
void CI1302_init(uint8 uart_index, uint32 baudrate, uint32 uart_tx_pin, uint32 uart_rx_pin)
{
    ci1302_uart_index = uart_index;

    fifo_init(&uart_data_fifo, FIFO_DATA_8BIT, uart_get_data, 64);
    uart_init(uart_index, baudrate, uart_tx_pin, uart_rx_pin);
    uart_rx_interrupt(uart_index, 1);

}

/**
 * @brief 处理FIFO中待解析的串口数据（应在主循环中调用）
 */
void CI1302_process_fifo_data(void)
{
    fifo_data_count = fifo_used(&uart_data_fifo);
    if(fifo_data_count != 0)
    {
        fifo_read_buffer(&uart_data_fifo, fifo_get_data, &fifo_data_count, FIFO_READ_AND_CLEAN);
        uart_stream_parser();
    }
}

/**
 * @brief UART接收中断处理函数，将接收的字节写入FIFO
 */
void CI1302_uart_rx_interrupt_handler(void)
{
    if(uart_query_byte(ci1302_uart_index, &get_data))
    {
        fifo_write_buffer(&uart_data_fifo, &get_data, 1);
    }
}

#pragma section all restore
