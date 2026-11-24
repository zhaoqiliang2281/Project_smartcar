#include "menu.h"

uint8 menu_system_mode = 0;  // 0=Menu, 1=KM1, 2=KM2, 3=KM3, 4=DipSw
uint8 KM2_edit_active = 0;    // KM2 GPS路点编辑模式激活标志
uint8 km2_submode = 0;        // KM2子模式: 0=OFF, 1=Record, 2=Run, 3=Correct
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

// =========================
// 系统模式变量
// =========================
// (defined above: uint8 menu_system_mode = 0)

// =========================
// 长按退出辅助函数
// =========================
extern MenuPage sub2_menu;  /* 前置声明, 定义在文件末尾 */

void menu_handle_long_press_exit(void)
{
    menu_key_clear_state(MENU_KEY_1);
    Back_Motor_SetPwm(0, 0);
    dispatch_car = 0;
    Angle_Target_Position = 0;
    Back_Motor_PID_Start_flag = 0;
    Motor_Start = 0;
    KM2_edit_active = 0;
    km2_submode = KM2_SUBMODE_OFF;
    Km2_protect_flag = 0;
    menu_system_mode = 0;
    ips200_clear();
    menu_init(&main_menu);
    /* 回到KM2子菜单, 不是主菜单根目录 */
    sub2_menu.parent = &main_menu;
    menu_current_page = &sub2_menu;
    menu_current_selection = 0;
    menu_full_redraw = 1;
}

// =========================
// KM1 动作函数 (科目一: GPS导航)
// =========================
void KM1_Record(void)
{
    KM_Select_flag = 1;
    menu_system_mode = 1;
    ips200_clear();
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    ips200_show_string(0, 16*0, "KM1: Record Mode");
    ips200_show_string(0, 16*2, "KEY1: Record GPS point");
    ips200_show_string(0, 16*3, "KEY1: Record Point");
    ips200_show_string(0, 16*4, "KEY2: Save Flash");
    ips200_show_string(0, 16*5, "KEY3: Load Flash");
    ips200_show_string(0, 16*6, "");
    ips200_show_string(0, 16*8, "Long K1: Back to Menu");
    Buzzer_check(50);
    menu_set_status("KM1 Recording Mode");
}

void KM1_EraseFlash(void)
{
    KM_Select_flag = 1;
    flash_erase_page(FLASH_SECTION_INDEX, FLASH_GPS_INDEX);
    menu_set_status("KM1: Flash Erased");
    Buzzer_check(50);
}

void KM1_ShowPath(void)
{
    KM_Select_flag = 1;
    next = 0;
    ips200_clear();
    gps_point gps_point_test[] =
    {
        {LAT_data_go[0], LOG_data_go[0]},
        {LAT_data_go[1], LOG_data_go[1]},
        {LAT_data_go[2], LOG_data_go[2]},
        {LAT_data_go[3], LOG_data_go[3]},
        {LAT_data_go[4], LOG_data_go[4]},
        {LAT_data_go[5], LOG_data_go[5]},
        {LAT_data_go[6], LOG_data_go[6]},
        {LAT_data_go[7], LOG_data_go[7]},
        {LAT_data_go[8], LOG_data_go[8]},
        {LAT_data_go[9], LOG_data_go[9]},
        {LAT_data_go[10], LOG_data_go[10]},
        {LAT_data_go[11], LOG_data_go[11]},
        {LAT_data_go[12], LOG_data_go[12]},
        {LAT_data_go[13], LOG_data_go[13]},
        {LAT_data_go[14], LOG_data_go[14]},
    };
    user_gps_display_init(IPS200_TYPE, 10, 10, 220, 300);
    user_gps_transition(gps_point_test, sizeof(gps_point_test) / sizeof(gps_point_test[0]));
    user_gps_display(RGB565_PURPLE);
    menu_set_status("KM1 Path Displayed - Long ENC Back");
    Buzzer_check(50);
}

void KM1_Run(void)
{
    KM_Select_flag = 1;
    next = 1;
    dispatch_car = 1;
    Motor_Start = 1;
    Back_Motor_Set_Speed = Speed_KM1_Buf[0];
    Back_Motor_SetPwm(Back_Motor_Set_Speed, Back_Motor_Set_Speed);
    menu_system_mode = 1;
    ips200_clear();
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    ips200_show_string(0, 16*0, "KM1: Running...");
    ips200_show_string(0, 16*2, "Long K1: Stop & Back");
    Buzzer_check(100);
    menu_set_status("KM1 Running");
}

void KM1_CorrectGPS(void)
{
    KM_Select_flag = 1;
    LAT_error = gnss.latitude - LAT_data_go[0];
    LOG_error = gnss.longitude - LOG_data_go[0];
    for(int i = 0; i < 15; i++)
    {
        LAT_data_go[i] += LAT_error;
        LOG_data_go[i] += LOG_error;
    }
    menu_set_status("KM1: GPS Corrected");
    Buzzer_check(50);
}

void KM1_EditPoints(void)
{
    KM_Select_flag = 1;
    menu_system_mode = 1;
    ips200_clear();
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    ips200_show_string(0, 16*0, "KM1: Edit Points");
    ips200_show_string(0, 16*2, "SW1 ON, SW2 OFF to edit");
    ips200_show_string(0, 16*3, "Long K1: Back to Menu");
    menu_set_status("KM1 Edit Points Mode");
}

void KM1_SpeedSet(void)
{
    KM_Select_flag = 1;
    menu_set_status("KM1 Speed: use Speed_KM1_Buf[]");
    Buzzer_check(50);
}

// =========================
// KM2 动作函数 (科目二: 灯光语音)
// =========================
void KM2_Record(void)
{
    KM_Select_flag = 2;
    menu_system_mode = 2;
    km2_submode = KM2_SUBMODE_RECORD;
    reset_gps_index();             // 每次进入录制模式从头开始
    ips200_full(RGB565_BLACK);
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    ips200_show_string(0, 16*0, "KM2: Record Mode");
    ips200_show_string(0, 16*2, "K1:Record K2:Save K3:Undo");
    ips200_show_string(0, 16*4, "Long K1: Back to Menu");
    Buzzer_check(50);
    menu_set_status("KM2 Recording Mode");
}

void KM2_SaveFlash(void)
{
    KM_Select_flag = 2;
    Up_flash_point();
    FLASH_Get_KM2();
    menu_set_status("KM2: Saved to Flash OK");
    Buzzer_check(50);
}

void KM2_LoadFlash(void)
{
    KM_Select_flag = 2;
    FLASH_READ_KM2();
    Read_flash_point();
    menu_set_status("KM2: Load from Flash OK");
    Buzzer_check(50);
}


void KM2_ShowPath(void)
{
    KM_Select_flag = 2;
    next = 0;
    ips200_clear();
    gps_point gps_point_test[] =
    {
        {LAT_data_go[0], LOG_data_go[0]},
        {LAT_data_go[1], LOG_data_go[1]},
        {LAT_data_go[2], LOG_data_go[2]},
        {LAT_data_go[3], LOG_data_go[3]},
        {LAT_data_go[4], LOG_data_go[4]},
        {LAT_data_go[5], LOG_data_go[5]},
        {LAT_data_go[6], LOG_data_go[6]},
        {LAT_data_go[7], LOG_data_go[7]},
        {LAT_data_go[8], LOG_data_go[8]},
        {LAT_data_go[9], LOG_data_go[9]},
        {LAT_data_go[10], LOG_data_go[10]},
        {LAT_data_go[11], LOG_data_go[11]},
        {LAT_data_go[12], LOG_data_go[12]},
        {LAT_data_go[13], LOG_data_go[13]},
        {LAT_data_go[14], LOG_data_go[14]},
        {LAT_data_go[15], LOG_data_go[15]},
    };
    user_gps_display_init(IPS200_TYPE, 10, 10, 220, 300);
    user_gps_transition(gps_point_test, sizeof(gps_point_test) / sizeof(gps_point_test[0]));
    user_gps_display(RGB565_YELLOW);
    menu_set_status("KM2 Path Displayed - Long ENC Back");
    Buzzer_check(50);
}

void KM2_Run(void)
{
    KM_Select_flag = 2;
    km2_submode = KM2_SUBMODE_RUN;
    /* 初始化运行所需的变量 (但不自动触发 Km2_protect_flag) */
    next = 1;
    Motor_Start = 1;
    Back_Motor_Set_Speed = Speed_KM2_Buf[0];
    Go_Flag = 1;
    Back_Flag = 0;
    /* 进入 KM2 模式, 等待语音指令 + 手动 KEY2 触发 */
    menu_system_mode = 2;
    ips200_clear();
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    ips200_show_string(0, 16*0, "KM2: Waiting Voice Cmd");
    ips200_show_string(0, 16*1, "Speak commands...");
    ips200_show_string(0, 16*3, "KEY2: Start Run  KEY3: Undo");
    ips200_show_string(0, 16*4, "KEY4: GPS Correct");
    ips200_show_string(0, 16*6, "Long K1: Back to Menu");
    Buzzer_check(50);
    menu_set_status("KM2 Ready - Wait for Cmd");
}

void KM2_CorrectGPS(void)
{
    KM_Select_flag = 2;
    km2_submode = KM2_SUBMODE_CORRECT;
    menu_system_mode = 2;
    ips200_clear();
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    ips200_show_string(0, 16*0, "KM2: Correct GPS Mode");
    ips200_show_string(0, 16*2, "KEY3: GPS Offset Correct");
    ips200_show_string(0, 16*3, "KEY4: Preset Cmd Sequence");
    ips200_show_string(0, 16*5, "Long K1: Back to Menu");
    Buzzer_check(50);
    menu_set_status("KM2 Correct GPS Mode");
}

void KM2_EditPoints(void)
{
    KM_Select_flag = 2;
    KM2_edit_active = 1;          /* 直接激活路点编辑器 */
    menu_system_mode = 2;
    ips200_clear();
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    ips200_show_string(0, 16*0, "KM2: Edit Points");
    ips200_show_string(0, 16*1, "Encoder: Browse/Edit");
    ips200_show_string(0, 16*2, "Btn: Mode K2:Save K3:Mark");
    ips200_show_string(0, 16*4, "Long K1: Back to Menu");
    Buzzer_check(50);
    menu_set_status("KM2 Edit Points Mode");
}

void KM2_LightTest(void)
{
    KM_Select_flag = 2;
    ips200_clear();
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    ips200_show_string(0, 16*0, "KM2: Light Test");
    ips200_show_string(0, 16*2, "Testing all lights...");
    emergency_flasher();
    system_delay_ms(500);
    left_turn_signal();
    system_delay_ms(500);
    Right_turn_signal();
    system_delay_ms(500);
    dipped_headlight();
    system_delay_ms(500);
    headlights_on_full_beam();
    system_delay_ms(500);
    foglight();
    system_delay_ms(500);
    position_light();
    system_delay_ms(500);
    windshield_wiper_on();
    menu_set_status("KM2: Light Test Done");
    Buzzer_check(100);
}

void KM2_VoiceTest(void)
{
    KM_Select_flag = 2;
    BY9301_Say(1);
    BY9301_setVolume(25);
    menu_set_status("KM2: Voice Test Done");
    Buzzer_check(50);
}

void KM2_SpeedSet(void)
{
    KM_Select_flag = 2;
    menu_set_status("KM2 Speed: use Speed_KM2_Buf[]");
    Buzzer_check(50);
}

// =========================
// 拨码开关模式进入
// =========================
void dip_switch_mode_enter(void)
{
    menu_system_mode = 4;
    ips200_clear();
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    ips200_show_string(0, 16*0, "Dip Switch Mode");
    ips200_show_string(0, 16*2, "SW1/2: Select Subject");
    ips200_show_string(0, 16*3, "SW+KEY: Operate");
    ips200_show_string(0, 16*5, "Long K1: Back to Menu");
    menu_set_status("Dip Switch Mode Active");
    Buzzer_check(50);
}

// =========================
// 拨码开关模式任务 (主循环调用)
// =========================
void dip_switch_mode_task(void)
{
    /* 检查长按退出 */
    if(MENU_KEY_LONG_PRESS == menu_key_get_state(MENU_KEY_1))
    {
        menu_handle_long_press_exit();
        return;
    }

    /* 原始拨码开关模式选择逻辑 */
    if(!gpio_get_level(sw1) && gpio_get_level(sw2))  // sw1 ON, sw2 OFF
    {
        if(key1_flag == 1)
        {
            key1_flag = 0;
            ips200_clear();
            Buzzer_check(50);
            if(Km13 == 0)
            {
                Km13 = 1;
                KM_Select_flag = 1;     // 选择KM1
            }
            else
            {
                Km13 = 0;
                KM_Select_flag = 3;     // 选择KM3
            }
        }
        if(key2_flag == 1)
        {
            key2_flag = 0;
            KM_Select_flag = 2;         // 选择KM2
            Buzzer_check(50);
        }
    }

    /* 根据选择的模式运行 */
    switch(KM_Select_flag)
    {
        case 1:    // KM1
            ips200_show_string(195, 16*8, "KM1");
            System_Setting_KM1();
            if(Transform == 1)
            {
                Follow_track1();
            }
            break;

        case 2:    // KM2
            ips200_show_string(195, 16*8, "KM2");
            System_NSetting_KM2();
            CI1302_process_fifo_data();
            KM2_StateMachine();               /* 公共状态机 */
            break;

        case 3:    // KM3 - 由主循环 INS_choise 处理
            break;
    }
}

// ========== KM1 (科目一) 子菜单 ==========
MenuItem sub1_items[] = {
    {"1. Record",        KM3_Record,    0},
    {"2. Save Flash",    KM1_SaveFlash, 0},
    {"3. Load Flash",    KM1_LoadFlash, 0},
    {"4. Edit Points",   KM3_Editor,    0},
    {"5. Speed Set",     KM3_SpeedSet,  0},
    {"6. GPS->INS",      KM3_Correct,   0},
    {"7. GPS Replace",   KM3_Replace,   0},
    {"8. Run",           KM1_Run_INS,   0},
    {"9. DiffGain",      KM3_Stop,      0},
    {"A. Gain",          KM3_Gain,      0},
    {"B. GPS Mode",      KM3_GPSMode,   0},
    {"C. Reverse Path",  KM3_ReversePath,0},
    {"D. Disp Path",    KM3_DisplayPath,0},
};

MenuPage sub1_menu = {
    "KM1-INS",
    sub1_items,
    sizeof(sub1_items) / sizeof(sub1_items[0]),
    0
};

// ========== KM2 (科目二) 子菜单 ==========
MenuItem sub2_items[] = {
    {"1. Record",        KM2_Record,    0},
    {"2. Save Flash",    KM2_SaveFlash, 0},
    {"3. Load Flash",    KM2_LoadFlash, 0},
    {"4. Show Path",     KM2_ShowPath,  0},
    {"5. Run",           KM2_Run,       0},
    {"6. Correct GPS",   KM2_CorrectGPS,0},
    {"7. Edit Points",   KM2_EditPoints,0},
    {"8. Light Test",    KM2_LightTest, 0},
    {"9. Voice Test",    KM2_VoiceTest, 0},
    {"A. Speed Set",     KM2_SpeedSet,  0},

    {"B. Action Test",  KM2_ActionTest,0},
    {"C. EditSeq",      KM2_EditSequence,0},
    {"D. NavMode",      KM2_NavMode,    0},
};

MenuPage sub2_menu = {
    "KM2-Light",
    sub2_items,
    sizeof(sub2_items) / sizeof(sub2_items[0]),
    0
};

// ========== PID 菜单 ==========
static void action_test_1(void) { menu_set_status("Action 1 OK"); }
static void action_test_2(void) { menu_set_status("Action 2 OK"); }

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

// ========== KM3 菜单 ==========
MenuItem sub3_items[] = {
    {"1. Record",        KM3_Record,   0},
    {"2. Save Flash",    KM3_SaveFlash, 0},
    {"3. Load Flash",    KM3_LoadFlash, 0},
    {"4. Edit Points",   KM3_Editor,   0},
    {"5. Speed Set",     KM3_SpeedSet, 0},
    {"6. GPS->INS",      KM3_Correct,  0},
    {"7. GPS Replace",   KM3_Replace,  0},
    {"8. Run",           KM3_Run,      0},
    {"9. DiffGain",      KM3_Stop,     0},
    {"A. Gain",         KM3_Gain,     0},
    {"B. GPS Mode",     KM3_GPSMode,  0},
    {"C. Reverse Path", KM3_ReversePath, 0},
    {"D. Disp Path",    KM3_DisplayPath,0},
};

MenuPage sub3_menu = {
    "KM3-INS",
    sub3_items,
    sizeof(sub3_items) / sizeof(sub3_items[0]),
    0
};

// ========== 主菜单 ==========
MenuItem main_items[] = {
    {"1. DipSw Mode",   dip_switch_mode_enter, 0},
    {"2. KM1-Navi",     0, &sub1_menu},
    {"3. KM2-Light",    0, &sub2_menu},
    {"4. KM3-INS",      0, &sub3_menu},
    {"5. PID",          0, &pid_menu},
};

MenuPage main_menu = {
    "Main Menu",
    main_items,
    sizeof(main_items) / sizeof(main_items[0]),
    0
};
