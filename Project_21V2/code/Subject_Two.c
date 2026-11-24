/*
 * Subject_Two.c
 *
 *  Created on: 2026年4月3日
 *      Author: liuliangfang
 */
#include "zf_common_headfile.h"

/*测试灯光*/
//        emergency_flasher();
//        system_delay_ms(1000);           // 等待后继续执行下一指令
//        left_turn_signal();
//        system_delay_ms(1000);           // 等待后继续执行下一指令
//        dipped_headlight();
//        system_delay_ms(1000);           // 等待后继续执行下一指令
//
//        headlights_on_full_beam();
//        system_delay_ms(1000);           // 等待后继续执行下一指令
//        Right_turn_signal();
//        system_delay_ms(1000);           // 等待后继续执行下一指令
//        foglight();
//        system_delay_ms(1000);           // 等待后继续执行下一指令
//        windshield_wiper_on();
//        system_delay_ms(1000);           // 等待后继续执行下一指令
//        position_light();
//        system_delay_ms(1000);           // 等待后继续执行下一指令


// 执行状态
volatile uint8_t cmd_sequence_running = 0;


void run_command_sequence(void)
{
    if (data_store_count == 0) {
        // 没有指令需要执行
        return;
    }

    cmd_sequence_running = 1;

    for (uint32_t i = 0; i < data_store_count; i++) {
        uint8_t cmd_id = data_store[i];

        ips200_show_uint(8, 16*7, i+1, 1);         // 显示当前指令序号
        ips200_show_uint(8*3, 16*7, cmd_id, 2);    // 显示当前指令 ID

//        const char *cmd_name = CI1302_get_command_name(cmd_id);
//        ips200_show_string(0, 16*8, (char *)cmd_name);

        // 执行硬件指令（每个指令循环只执行一次）
        execute_command(cmd_id);

        // 如果不是最后一条指令，延时 1 秒
        if (i != data_store_count - 1) {
            system_delay_ms(1000);
        }
    }

    // 执行完成后清空存储
    data_store_count = 0;
    memset(data_store, 0, sizeof(data_store));

    // 刷新屏幕显示
//    ips200_update_data_store_display();

    cmd_sequence_running = 0;
}

void emergency_flasher(void)
{
    // 设置亮度
    dot_matrix_screen_set_brightness(5000);
    //双闪灯
    dot_matrix_screen_data[0] = 0xFF;  // 双闪灯-左
    dot_matrix_screen_data[1] = 0xFF;  // 双闪灯-中间
    dot_matrix_screen_data[2] = 0xFF;  // 双闪灯-右
    // 双闪灯执行后等待一段时间再开始下一次
    system_delay_ms(1000);           // 等待后继续执行下一指令
}

void left_turn_signal(void)
{
    //左转
    dot_matrix_screen_set_brightness(5000);
    dot_matrix_screen_data[0] = 0xFE;  // 左转灯-左
    dot_matrix_screen_data[1] = 0xFE;  // 左转灯-中间
    dot_matrix_screen_data[2] = 0xFF;  // 左转灯-右
    system_delay_ms(1000);           // 等待后继续执行下一指令
}

void dipped_headlight(void)
{
    //近光灯
    dot_matrix_screen_set_brightness(5000);
    dot_matrix_screen_data[0] = 0xFD;  // 近光灯-左
    dot_matrix_screen_data[1] = 0xFD;  // 近光灯-中间
    dot_matrix_screen_data[2] = 0xFD;  // 近光灯-右
    system_delay_ms(1000);           // 等待后继续执行下一指令
}

void headlights_on_full_beam(void)
{
    //远光灯
    dot_matrix_screen_set_brightness(5000);
    dot_matrix_screen_data[0] = 0xFC;  // 远光灯-左
    dot_matrix_screen_data[1] = 0xFD;  // 远光灯-中间
    dot_matrix_screen_data[2] = 0xFD;  // 远光灯-右
    system_delay_ms(1000);           // 等待后继续执行下一指令
}

void Right_turn_signal(void)
{
    //右转
    dot_matrix_screen_set_brightness(5000);
    dot_matrix_screen_data[0] = 0xFF;  // 右转灯-左
    dot_matrix_screen_data[1] = 0xFE;  // 右转灯-中间
    dot_matrix_screen_data[2] = 0xFE;  // 右转灯-右
    system_delay_ms(1000);           // 等待后继续执行下一指令
}

void foglight(void)
{
    // 雾灯
    dot_matrix_screen_set_brightness(5000);
    dot_matrix_screen_data[0] = 0xFB;  // 雾灯-左
    dot_matrix_screen_data[1] = 0xFD;  // 雾灯-中间
    dot_matrix_screen_data[2] = 0xFD;  // 雾灯-右
    system_delay_ms(1000);           // 等待后继续执行下一指令
}

void windshield_wiper_on(void)
{
    // 雨刷(左)用Y型图标表示 0xFA
    dot_matrix_screen_set_brightness(5000);
    dot_matrix_screen_data[0] = 0xFA;
    dot_matrix_screen_data[1] = 0xFA;
    dot_matrix_screen_data[2] = 0xFA;
    system_delay_ms(1000);
}

void position_light(void)
{
    // 示廓灯
    dot_matrix_screen_set_brightness(5000);
    dot_matrix_screen_data[0] = 0xF9;
    dot_matrix_screen_data[1] = 0xF9;
    dot_matrix_screen_data[2] = 0xF9;
    system_delay_ms(1000);
}

void whistle_repeat(uint8_t times, uint16_t on_ms, uint16_t off_ms)
{
//    for (uint8_t i = 0; i < times; i++) {
//        Buzzer_ON();
//        system_delay_ms(on_ms);
//        Buzzer_OFF();
//        if (i < times - 1) {
//            system_delay_ms(off_ms);
//        }
//    }
}

// 模拟指令处理函数    基本思路
void Process_Command(uint8_t cmd)
{
    switch(cmd)
    {

//        case 1:
//            //当前直行十秒
//            break;
//        case 2:
//            //后退直行十秒
//            break;
        case 3:
            emergency_flasher();
            break;
        case 4:
            left_turn_signal();
            break;
        case 5:
            Right_turn_signal();
            break;
        case 6:
            dipped_headlight();
            break;
        case 7:
            headlights_on_full_beam();
            break;
        case 8:
            foglight();
            break;
//
//        case 9: //当前左转十秒
//
//            break;
//        case 10: //当前右转十秒
//
//            break;
        case 11: //顺时针转一圈
            Right_circlet();
            break;
        case 12: //逆时针转一圈
            Left_circlet();
            break;
//        case 13: //停车等一秒
//            break;
//        case 14: //停车等三秒
//            break;
//        case 15: //停车等五秒
//            break;
//        case 18: //
//            break;
//        case 19: //
//            break;
//        case 20: //
//            break;
//        case 21: //
//            break;
//
//        case 42: // 获取经纬度2 - 经纬度获取
//
//            break;
        case 31: // 打开雨刷
            windshield_wiper_on();
            break;
        case 32: // 语音播报1遍
//            Whistle_time();
            BY9301_Say(1);
            BY9301_setVolume(25);
            break;
        case 33: // 语音播报2遍
//            Whistle_time();
            BY9301_Say(2);
            BY9301_setVolume(25);
            break;
        case 34: // 语音播报3遍
//            Whistle_time();
            BY9301_Say(3);
            BY9301_setVolume(25);
            break;
        case 35: // 语音播报4遍
            BY9301_Say(4);
            BY9301_setVolume(25);
            break;
        case 36: // 语音播报5遍
            BY9301_Say(5);
            BY9301_setVolume(25);
            break;
        case 37: // 语音播报6遍
            BY9301_Say(6);
            BY9301_setVolume(25);
            break;
        case 38: // 语音播报7遍
            BY9301_Say(7);
            BY9301_setVolume(25);
            break;
        case 39: // 语音播报8遍
            BY9301_Say(8);
            BY9301_setVolume(25);
            break;
        case 40: // 语音播报9遍
            BY9301_Say(9);
            BY9301_setVolume(25);
            break;
    }
}
