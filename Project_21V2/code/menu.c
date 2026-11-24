#include "zf_common_headfile.h"
// =========================
// 按键驱动
// =========================
static uint32 menu_key_scanner_period = 0;
static uint32 menu_key_press_time[MENU_KEY_NUMBER];
static menu_key_state_enum menu_key_state[MENU_KEY_NUMBER];
static const gpio_pin_enum menu_key_pin_map[MENU_KEY_NUMBER] = MENU_KEY_LIST;
static uint8 menu_key_long_press_flag[MENU_KEY_NUMBER] = {1, 1, 1, 1};

void menu_key_scanner(void)
{
    uint8 i;

    for (i = 0; i < MENU_KEY_NUMBER; i++)
    {
        if (MENU_KEY_RELEASE_LEVEL != gpio_get_level(menu_key_pin_map[i]))
        {
            menu_key_press_time[i]++;

            if (0 == menu_key_long_press_flag[i])
            {
                menu_key_state[i] = MENU_KEY_RELEASE;
            }
            else if ((MENU_KEY_LONG_PRESS_PERIOD / menu_key_scanner_period) <= menu_key_press_time[i])
            {
                menu_key_state[i] = MENU_KEY_LONG_PRESS;
                menu_key_long_press_flag[i] = 0;
            }
        }
        else
        {
            if (1 == menu_key_long_press_flag[i])
            {
                if ((MENU_KEY_LONG_PRESS != menu_key_state[i]) &&
                    ((MENU_KEY_MAX_SHOCK_PERIOD / menu_key_scanner_period) <= menu_key_press_time[i]))
                {
                    menu_key_state[i] = MENU_KEY_SHORT_PRESS;
                }
                else
                {
                    menu_key_state[i] = MENU_KEY_RELEASE;
                }
            }

            menu_key_press_time[i] = 0;
            menu_key_long_press_flag[i] = 1;
        }
    }
}

menu_key_state_enum menu_key_get_state(menu_key_index_enum key_n)
{
    return menu_key_state[key_n];
}

void menu_key_clear_state(menu_key_index_enum key_n)
{
    menu_key_state[key_n] = MENU_KEY_RELEASE;
}

void menu_key_clear_all_state(void)
{
    uint8 i;
    for (i = 0; i < MENU_KEY_NUMBER; i++)
    {
        menu_key_state[i] = MENU_KEY_RELEASE;
    }
}

void menu_key_init(uint32 period)
{
    uint8 i;
    zf_assert(0 < period);

    for (i = 0; i < MENU_KEY_NUMBER; i++)
    {
        gpio_init(menu_key_pin_map[i], GPI, GPIO_HIGH, GPI_PULL_UP);
        menu_key_state[i] = MENU_KEY_RELEASE;
        menu_key_press_time[i] = 0;
        menu_key_long_press_flag[i] = 1;
    }

    menu_key_scanner_period = period;
}

// =========================
// 编码器驱动
// =========================
static int menu_encoder_change_num = 0;
static int menu_encoder_pending_steps = 0;
static int menu_encoder_raw_accum = 0;
static uint8 menu_encoder_prev_state = 0xFF;

#define MENU_ENCODER_COUNTS_PER_STEP   2
#define MENU_ENCODER_PENDING_LIMIT     8
#define MENU_ENCODER_DIR_SIGN         -1

static int8 menu_encoder_transition_table[16] =
{
     0, -1,  1,  0,
     1,  0,  0, -1,
    -1,  0,  0,  1,
     0,  1, -1,  0
};

static uint8 menu_encoder_read_state(void)
{
    uint8 a_level = (uint8)gpio_get_level(MENU_ENCODER_A_PIN);
    uint8 b_level = (uint8)gpio_get_level(MENU_ENCODER_B_PIN);
    return (uint8)((a_level << 1) | b_level);
}

void menu_encoder_init(void)
{
    gpio_init(MENU_ENCODER_A_PIN, GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(MENU_ENCODER_B_PIN, GPI, GPIO_HIGH, GPI_PULL_UP);

    menu_encoder_change_num = 0;
    menu_encoder_pending_steps = 0;
    menu_encoder_raw_accum = 0;
    menu_encoder_prev_state = menu_encoder_read_state();
}

void menu_encoder_update(void)
{
    uint8 current_state = menu_encoder_read_state();
    uint8 transition_index;
    int8 delta;

    if (menu_encoder_prev_state > 3U)
    {
        menu_encoder_prev_state = current_state;
        return;
    }

    if (current_state == menu_encoder_prev_state)
    {
        return;
    }

    transition_index = (uint8)((menu_encoder_prev_state << 2) | current_state);
    delta = menu_encoder_transition_table[transition_index];
    menu_encoder_prev_state = current_state;

    if (0 == delta)
    {
        return;
    }

    delta = (int8)(delta * MENU_ENCODER_DIR_SIGN);
    menu_encoder_raw_accum += delta;

    if (menu_encoder_raw_accum >= MENU_ENCODER_COUNTS_PER_STEP)
    {
        if (menu_encoder_pending_steps < MENU_ENCODER_PENDING_LIMIT)
        {
               menu_encoder_pending_steps++;
        }
        menu_encoder_raw_accum = 0;
    }
    else if (menu_encoder_raw_accum <= -MENU_ENCODER_COUNTS_PER_STEP)
    {
        if (menu_encoder_pending_steps > -MENU_ENCODER_PENDING_LIMIT)
        {
                menu_encoder_pending_steps--;
        }
        menu_encoder_raw_accum = 0;
    }
}

uint8 menu_encoder_has_change(void)
{
    return (0 != menu_encoder_pending_steps);
}

int menu_encoder_get_change(void)
{
    if (menu_encoder_pending_steps > 0)
    {
        menu_encoder_pending_steps--;
        menu_encoder_change_num = 1;
        return menu_encoder_change_num;
    }
    else if (menu_encoder_pending_steps < 0)
    {
        menu_encoder_pending_steps++;
        menu_encoder_change_num = -1;
        return menu_encoder_change_num;
    }

    menu_encoder_change_num = 0;
    return 0;
}

// =========================
// 菜单显示层
// =========================
static MenuPage *menu_current_page = 0;
static int menu_current_selection = 0;
static int menu_last_selection = -1;
static uint8 menu_need_update = 1;
static uint8 menu_full_redraw = 1;
static const char *menu_status_text = 0;

static void menu_fill_rect(uint16 x_start, uint16 y_start, uint16 x_end, uint16 y_end, uint16 color)
{
    uint16 y;

    if (x_start >= ips200_width_max) x_start = ips200_width_max - 1;
    if (x_end >= ips200_width_max) x_end = ips200_width_max - 1;
    if (y_start >= ips200_height_max) y_start = ips200_height_max - 1;
    if (y_end >= ips200_height_max) y_end = ips200_height_max - 1;

    if (x_start > x_end) { uint16 t = x_start; x_start = x_end; x_end = t; }
    if (y_start > y_end) { uint16 t = y_start; y_start = y_end; y_end = t; }

    for (y = y_start; y <= y_end; y++)
    {
        ips200_draw_line(x_start, y, x_end, y, color);
    }
}

static void menu_show_string_fit(uint16 x, uint16 y, const char *s)
{
    int max_chars = (int)((ips200_width_max - 1 - x) / 8);
    char buf[64];
    int i = 0;

    if (max_chars <= 0) return;

    while (i < max_chars && s[i] != '\0' && i < (int)sizeof(buf) - 1)
    {
        buf[i] = s[i];
        i++;
    }
    buf[i] = '\0';
    ips200_show_string(x, y, buf);
}

static void menu_show_string_fit_width_pad(uint16 x, uint16 y, uint16 max_width, const char *s)
{
    int max_chars;
    char buf[64];
    int i = 0;

    if (0U == max_width) return;

    max_chars = (int)(max_width / 8U);
    if (max_chars <= 0) return;

    while (i < max_chars && s[i] != '\0' && i < (int)sizeof(buf) - 1)
    {
        buf[i] = s[i];
        i++;
    }

    while (i < max_chars && i < (int)sizeof(buf) - 1)
    {
        buf[i] = ' ';
        i++;
    }

    buf[i] = '\0';
    ips200_show_string(x, y, buf);
}

void menu_set_status(const char *text)
{
    menu_status_text = text;
    menu_need_update = 1;
    menu_full_redraw = 1;
}

MenuPage *menu_get_current_page(void)
{
    return menu_current_page;
}

int menu_get_current_index(void)
{
    return menu_current_selection;
}

void menu_init(MenuPage *root)
{
#if MENU_USE_IPS200_SPI
    ips200_init(IPS200_TYPE_SPI);
#else
    ips200_init(IPS200_TYPE_PARALLEL8);
#endif

    ips200_set_dir(IPS200_DEFAULT_DISPLAY_DIR);
    ips200_set_font(IPS200_DEFAULT_DISPLAY_FONT);
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    ips200_full(RGB565_BLACK);

    menu_key_init(MENU_SCAN_PERIOD_MS);
    menu_key_clear_all_state();
    menu_encoder_init();

    menu_current_page = root;
    if (menu_current_page)
    {
        menu_current_page->parent = 0;
    }
    menu_current_selection = 0;
    menu_last_selection = -1;
    menu_need_update = 1;
    menu_full_redraw = 1;
    menu_status_text = 0;
}

void menu_draw(void)
{
    int i;

    if (0 == menu_current_page || menu_current_page->num_items <= 0)
    {
        return;
    }

    if (menu_full_redraw)
    {
        ips200_full(RGB565_BLACK);

        ips200_set_color(RGB565_YELLOW, RGB565_BLACK);
        menu_show_string_fit(10, 10, menu_current_page->title);
        ips200_draw_line(0, 30, ips200_width_max - 1, 30, RGB565_GRAY);

        for (i = 0; i < menu_current_page->num_items; i++)
        {
            uint16 y_pos = (uint16)(MENU_ITEM_START_Y + i * MENU_ITEM_STEP_Y);
            if (y_pos + 16 >= ips200_height_max) break;

            if (i == menu_current_selection)
            {
                menu_fill_rect(5, (uint16)(y_pos - 2), (uint16)(ips200_width_max - 5), (uint16)(y_pos + 16), RGB565_WHITE);
                ips200_set_color(RGB565_BLACK, RGB565_WHITE);
                menu_show_string_fit(10, y_pos, menu_current_page->items[i].name);
            }
            else
            {
                ips200_set_color(RGB565_WHITE, RGB565_BLACK);
                menu_show_string_fit(10, y_pos, menu_current_page->items[i].name);
            }
        }

        ips200_set_color(RGB565_GRAY, RGB565_BLACK);
        menu_show_string_fit_width_pad(5, (uint16)(ips200_height_max - 20), (uint16)(ips200_width_max - 10), "ENC:Move K1:OK/LONG:BK");

        if (menu_status_text)
        {
            ips200_set_color(RGB565_CYAN, RGB565_BLACK);
            menu_show_string_fit_width_pad(5, (uint16)(ips200_height_max - 40), (uint16)(ips200_width_max - 10), menu_status_text);
        }

        menu_last_selection = menu_current_selection;
        menu_full_redraw = 0;
        menu_need_update = 0;
        return;
    }

    if (menu_need_update)
    {
        int prev = menu_last_selection;
        int curr = menu_current_selection;

        if (prev >= 0 && prev < menu_current_page->num_items)
        {
            uint16 y_prev = (uint16)(MENU_ITEM_START_Y + prev * MENU_ITEM_STEP_Y);
            if (y_prev + 16 < ips200_height_max)
            {
                menu_fill_rect(5, (uint16)(y_prev - 2), (uint16)(ips200_width_max - 5), (uint16)(y_prev + 16), RGB565_BLACK);
                ips200_set_color(RGB565_WHITE, RGB565_BLACK);
                menu_show_string_fit(10, y_prev, menu_current_page->items[prev].name);
            }
        }

        if (curr >= 0 && curr < menu_current_page->num_items)
        {
            uint16 y_curr = (uint16)(MENU_ITEM_START_Y + curr * MENU_ITEM_STEP_Y);
            if (y_curr + 16 < ips200_height_max)
            {
                menu_fill_rect(5, (uint16)(y_curr - 2), (uint16)(ips200_width_max - 5), (uint16)(y_curr + 16), RGB565_WHITE);
                ips200_set_color(RGB565_BLACK, RGB565_WHITE);
                menu_show_string_fit(10, y_curr, menu_current_page->items[curr].name);
            }
        }

        if (menu_status_text)
        {
            menu_fill_rect(0, (uint16)(ips200_height_max - 42), (uint16)(ips200_width_max - 1), (uint16)(ips200_height_max - 26), RGB565_BLACK);
            ips200_set_color(RGB565_CYAN, RGB565_BLACK);
            menu_show_string_fit_width_pad(5, (uint16)(ips200_height_max - 40), (uint16)(ips200_width_max - 10), menu_status_text);
        }

        menu_last_selection = menu_current_selection;
        menu_need_update = 0;
    }
}

void menu_task(void)
{
    int encoder_change;
    MenuItem *item;

    if (0 == menu_current_page || menu_current_page->num_items <= 0)
    {
        return;
    }

    system_delay_ms(MENU_SCAN_PERIOD_MS);
    menu_key_scanner();
    menu_encoder_update();

    if (menu_encoder_has_change())
    {
        encoder_change = menu_encoder_get_change();
        menu_current_selection -= encoder_change;

        while (menu_current_selection >= menu_current_page->num_items)
        {
            menu_current_selection -= menu_current_page->num_items;
        }

        while (menu_current_selection < 0)
        {
            menu_current_selection += menu_current_page->num_items;
        }

        menu_need_update = 1;
    }

    if (MENU_KEY_LONG_PRESS == menu_key_get_state(MENU_KEY_1))
    {
        menu_key_clear_state(MENU_KEY_1);
        if (menu_current_page->parent)
        {
            menu_current_page = menu_current_page->parent;
            menu_current_selection = 0;
            menu_status_text = 0;
            menu_need_update = 1;
            menu_full_redraw = 1;
        }

        menu_draw();
        return;
    }

    if (MENU_KEY_SHORT_PRESS == menu_key_get_state(MENU_KEY_1))
    {
        menu_key_clear_state(MENU_KEY_1);
        item = &menu_current_page->items[menu_current_selection];

        if (item->action)
        {
            item->action();
            menu_full_redraw = 1;
        }
        else if (item->sub_page)
        {
            item->sub_page->parent = menu_current_page;
            menu_current_page = item->sub_page;
            menu_current_selection = 0;
            menu_status_text = 0;
            menu_need_update = 1;
            menu_full_redraw = 1;
        }
    }

    menu_draw();
}


/***********************************************************/

// ========== 动作函数 ==========
static void action_test_1(void)
{
    menu_set_status("Action 1 OK");
}

static void action_test_2(void)
{
    menu_set_status("Action 2 OK");
}

//static void System_Setting_KM1(void)
//{
//    // 清屏并绘制固定界面（根据原逻辑可能需要绘制背景等）
//    ips200_full(RGB565_BLACK);
//    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
//    ips200_show_string(230, 16*8, "S");  // 显示 "S" 字符
//
//    // 主循环，直到长按 KEY1 退出
//    while (1)
//    {
//        // 1. 周期性按键扫描（必须，否则按键状态不会更新）
//        system_delay_ms(10);
//        menu_key_scanner();
//
//        // 2. 检查长按 KEY1 退出
//        if (MENU_KEY_LONG_PRESS == menu_key_get_state(MENU_KEY_1))
//        {
//            menu_key_clear_state(MENU_KEY_1);
//            break;  // 退出循环
//        }
//
//        // 3. 更新动态数据显示（如 GPS 信息等）
//        GPS_array();
//
//        // 4. 检查其他按键操作（KEY2/KEY3/KEY4）
//        //    注意：这里使用原全局标志 key2_flag 等，或者可以直接读取按键状态
//        if (key2_flag == 1)
//        {
//            key2_flag = 0;
//            Up_flash_point();
//            FLASH_Get_KM1();
//            Buzzer_check(50);
//        }
//        if (key3_flag == 1)
//        {
//            key3_flag = 0;
//            FLASH_READ_KM1();
//            Read_flash_point();
//            Buzzer_check(50);
//        }
//        if (key4_flag == 1)
//        {
//            key4_flag = 0;
//            flash_erase_page(FLASH_SECTION_INDEX, FLASH_GPS_INDEX);
//            Buzzer_check(50);
//            ips200_show_string(172, 16*12, "QI flash");
//        }
//
//        // 注意：如果原逻辑中 key2_flag 等是由中断或外部扫描设置的，请确保在菜单循环中能够更新
//        // 如果原来是用按键直接触发标志，现在可能需要在菜单的按键扫描中更新这些标志。
//        // 或者可以直接在 menu.c 中通过 menu_key_get_state(MENU_KEY_2) 来判断，而不依赖外部标志。
//        // 以下是直接使用菜单按键状态的版本（推荐，避免标志同步问题）：
//        /*
//        if (MENU_KEY_SHORT_PRESS == menu_key_get_state(MENU_KEY_2))
//        {
//            menu_key_clear_state(MENU_KEY_2);
//            Up_flash_point();
//            FLASH_Get_KM1();
//            Buzzer_check(50);
//        }
//        if (MENU_KEY_SHORT_PRESS == menu_key_get_state(MENU_KEY_3))
//        {
//            menu_key_clear_state(MENU_KEY_3);
//            FLASH_READ_KM1();
//            Read_flash_point();
//            Buzzer_check(50);
//        }
//        if (MENU_KEY_SHORT_PRESS == menu_key_get_state(MENU_KEY_4))
//        {
//            menu_key_clear_state(MENU_KEY_4);
//            flash_erase_page(FLASH_SECTION_INDEX, FLASH_GPS_INDEX);
//            Buzzer_check(50);
//            ips200_show_string(172, 16*12, "QI flash");
//        }
//        */
//    }
//
//    // 退出后可设置状态提示
//    menu_set_status("Return from KM1");
//}
//static void KM1_Show(void){
//
//
//    // 清屏并绘制固定界面（根据原逻辑可能需要绘制背景等）
//       ips200_full(RGB565_BLACK);
//       ips200_set_color(RGB565_WHITE, RGB565_BLACK);
//       ips200_show_string(230, 16*8, "X");
//       // 主循环，直到长按 KEY1 退出
//       while (1)
//       {
//           // 1. 周期性按键扫描（必须，否则按键状态不会更新）
//           system_delay_ms(10);
//           menu_key_scanner();
//
//           // 2. 检查长按 KEY1 退出
//           if (MENU_KEY_LONG_PRESS == menu_key_get_state(MENU_KEY_1))
//           {
//               menu_key_clear_state(MENU_KEY_1);
//               break;  // 退出循环
//           }
//
//           if(key1_flag == 1)
//           {
//               key1_flag=0;
//               next = 1;
//               gps_point gps_point_test[]=
//               {
//                   {LAT_data_go[0],LOG_data_go[0]},
//                   {LAT_data_go[1],LOG_data_go[1]},
//                   {LAT_data_go[2],LOG_data_go[2]},
//                   {LAT_data_go[3],LOG_data_go[3]},
//                   {LAT_data_go[4],LOG_data_go[4]},
//                   {LAT_data_go[5],LOG_data_go[5]},
//                   {LAT_data_go[6],LOG_data_go[6]},
//                   {LAT_data_go[7],LOG_data_go[7]},
//
//                   {LAT_data_go[8],LOG_data_go[8]},
//                   {LAT_data_go[9],LOG_data_go[9]},
//                   {LAT_data_go[10],LOG_data_go[10]},
//                   {LAT_data_go[11],LOG_data_go[11]},
//                   {LAT_data_go[12],LOG_data_go[12]},
//                   {LAT_data_go[13],LOG_data_go[13]},
//
//               };
//               user_gps_display_init(IPS200_TYPE,10,10,220,300);
//               user_gps_transition(gps_point_test,sizeof(gps_point_test) / sizeof(gps_point_test[0]));
//               user_gps_display(RGB565_PURPLE);
//               Buzzer_check(50);
//           }
//
//       }
//
//       // 退出后可设置状态提示
//       menu_set_status("Return from KM1");
//
//}
//static void KM1_Run(void){
//
//    if(key2_flag == 1)
//       {
//           key2_flag=0;
//           Buzzer_SetTimer(50);
//           delay_Start = 1;    //开启发车1s倒计时
//       }
//       if(Timer_delay_ms(1000)==1) //不堵塞式等待1s倒计时完成
//       {
//           dispatch_car = 1;   //发车标志位
//           delay_ScanTemp = 0; //清除置1标志位
//       }
//       if(dispatch_car == 1 && next == 1 && Transform == 1)  //发车起始速度
//       {
//           Back_Motor_SetPwm(200,200);
//       }
//       if(Transform == 1)   //Transform自动或手动标志位
//       {
//           Follow_track1();
//       }
//}
//static void Data_Display(void){
//
//    // 清屏并绘制固定界面（根据原逻辑可能需要绘制背景等）
//       ips200_full(RGB565_BLACK);
//       ips200_set_color(RGB565_WHITE, RGB565_BLACK);
//       ips200_show_string(0, 16*0, "Data_Display");
//       // 主循环，直到长按 KEY1 退出
//       while (1)
//       {
//           // 1. 周期性按键扫描（必须，否则按键状态不会更新）
//           system_delay_ms(10);
//           menu_key_scanner();
//
//           // 2. 检查长按 KEY1 退出
//           if (MENU_KEY_LONG_PRESS == menu_key_get_state(MENU_KEY_1))
//           {
//               menu_key_clear_state(MENU_KEY_1);
//               break;  // 退出循环
//           }
//
//           /*陀螺仪角度显示*/
//           ips200_show_string(0, 16*10, "Yaw:");ips200_show_float(30, 16*10, Yaw, 3, 6);
//           ips200_show_string(0, 16*11, "yaw:");ips200_show_float(30, 16*11, yaw, 3, 6);
//
//
//       }
//
//       // 退出后可设置状态提示
//       menu_set_status("Return from KM1");
//
//
//}
//
void sub2_run(void){


        // 清屏并绘制固定界面（根据原逻辑可能需要绘制背景等）
           ips200_full(RGB565_BLACK);
           ips200_set_color(RGB565_WHITE, RGB565_BLACK);
           ips200_show_string(0, 16*0, "su2_run");
           // 主循环，直到长按 KEY1 退出
           while (1)
           {
               // 1. 周期性按键扫描（必须，否则按键状态不会更新）
               system_delay_ms(10);
               menu_key_scanner();

               // 2. 检查长按 KEY1 退出
               if (MENU_KEY_LONG_PRESS == menu_key_get_state(MENU_KEY_1))
               {
                   menu_key_clear_state(MENU_KEY_1);
                   break;  // 退出循环
               }

//               key_scan();
//               Send_Flag_Get();
//               Send_Voice();
//               Wifi_Uart_message_parse();
//
//               if (!cmd_sequence_running && key3_flag==1)
//               {
//                   key3_flag=0;
//                   run_command_sequence();
//               }
//
//               ips200_show_uint(8, 16*2, pending_command[0], 2);
//               ips200_show_uint(8, 16*3, pending_command[1], 2);
//               ips200_show_uint(8, 16*4, pending_command[2], 2);
//               ips200_show_uint(8, 16*5, pending_command[3], 2);
//               ips200_show_uint(8, 16*6, pending_command[4], 2);
           }

           // 退出后可设置状态提示
           menu_set_status("Return from KM2");

}


// ========== (科目一)子菜单 1 ==========
MenuItem sub1_items[] = {
//    {"1. Edit Sampling_Point", System_Setting_KM1, 0},
//    {"2. Edit show", KM1_Show, 0},
//    {"3. Edit runs", KM1_Run, 0},
        {"1. Edit Sampling_Point", 0, 0},
        {"2. Edit show", 0, 0},
        {"3. Edit runs", 0, 0},
};

MenuPage sub1_menu = {
    "sub1",
    sub1_items,
    sizeof(sub1_items) / sizeof(sub1_items[0]),
    0
};
// ========== (科目二)子菜单 1 ==========
MenuItem sub2_items[] = {
    {"1. sub2_init", 0, 0},
    {"2. sub2_run", sub2_run, 0},
};

MenuPage sub2_menu = {
    "sub2",
    sub2_items,
    sizeof(sub2_items) / sizeof(sub2_items[0]),
    0
};

// ========== PID 菜单 ==========
MenuItem pid_items[] = {
    {"1. Edit Kp", action_test_1, 0},
    {"2. Edit Ki", action_test_2, 0},
};

MenuPage pid_menu = {
    "PID",
    pid_items,
    sizeof(pid_items) / sizeof(pid_items[0]),
    0
};

// ========== 科目选择菜单 ==========
MenuItem Subject_selection_items[] = {
    {"1. Edit sub1", 0, &sub1_menu},    // ? 此时 sub1_menu 已定义
    {"2. Edit sub2", 0, &sub2_menu},
    {"3. Edit sub3", 0, 0},
    {"4. Edit sub4", 0, 0},
};

MenuPage Subject_selection = {
    "Subject_selection",
    Subject_selection_items,
    sizeof(Subject_selection_items) / sizeof(Subject_selection_items[0]),
    0
};

//MenuItem Data_Display_items[] = {
//    {"1. Edit data", 0, 0},    // ? 此时 sub1_menu 已定义
//    {"2. Edit sub2", 0, &sub2_menu},
//
//};
//MenuPage Data_Display = {
//    "Data_Display",
//    Data_Display_items,
//    sizeof(Data_Display_items) / sizeof(Data_Display_items[0]),
//    0
//};

// ========== 主菜单 ==========
MenuItem main_items[] = {
    {"1. PID", 0, &pid_menu},
    {"2. Subject_selection", 0, &Subject_selection},
    //{"3. Data_Display", Data_Display, 0},
};

MenuPage main_menu = {
    "Main",
    main_items,
    sizeof(main_items) / sizeof(main_items[0]),
    0
};
