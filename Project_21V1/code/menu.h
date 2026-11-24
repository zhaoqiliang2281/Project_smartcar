#ifndef MENU_H
#define MENU_H

#include "zf_common_headfile.h"

// =========================
// 按键驱动
// =========================
#define MENU_KEY_LIST                    {P11_11}    // 旋转编码器按键
#define MENU_KEY_RELEASE_LEVEL           (GPIO_HIGH)
#define MENU_KEY_MAX_SHOCK_PERIOD        (20)
#define MENU_KEY_LONG_PRESS_PERIOD       (250)

typedef enum
{
    MENU_KEY_1,
    MENU_KEY_2,
    MENU_KEY_3,
    MENU_KEY_4,
    MENU_KEY_NUMBER,
} menu_key_index_enum;

typedef enum
{
    MENU_KEY_RELEASE,
    MENU_KEY_SHORT_PRESS,
    MENU_KEY_LONG_PRESS,
} menu_key_state_enum;

// =========================
// 编码器驱动
// =========================
#define MENU_ENCODER_A_PIN               P13_2
#define MENU_ENCODER_B_PIN               P21_6

// =========================
// 菜单结构
// =========================
typedef struct MenuPage MenuPage;
typedef struct MenuItem MenuItem;
typedef void (*menu_action_t)(void);

struct MenuItem
{
    const char *name;
    menu_action_t action;
    MenuPage *sub_page;
};

struct MenuPage
{
    const char *title;
    MenuItem *items;
    int num_items;
    MenuPage *parent;
};

// =========================
// 用户可选项定义
// =========================
#ifndef MENU_USE_IPS200_SPI
#define MENU_USE_IPS200_SPI              1
#endif

#ifndef MENU_SCAN_PERIOD_MS
#define MENU_SCAN_PERIOD_MS              10
#endif

#ifndef MENU_ITEM_START_Y
#define MENU_ITEM_START_Y                40
#endif

#ifndef MENU_ITEM_STEP_Y
#define MENU_ITEM_STEP_Y                 20
#endif

// =========================
// 公共函数
// =========================
void menu_init(MenuPage *root);
void menu_task(void);
void menu_draw(void);
void menu_set_status(const char *text);

MenuPage *menu_get_current_page(void);
int menu_get_current_index(void);

// =========================
// 按键驱动接口（一般不需要外部调用）
// =========================
void menu_key_init(uint32 period);
void menu_key_scanner(void);
menu_key_state_enum menu_key_get_state(menu_key_index_enum key_n);
void menu_key_clear_state(menu_key_index_enum key_n);
void menu_key_clear_all_state(void);

void menu_encoder_init(void);
void menu_encoder_update(void);
uint8 menu_encoder_has_change(void);
int menu_encoder_get_change(void);
/***********************************/
extern MenuPage main_menu;

// =========================
// 系统模式 (0=Menu, 1=KM1, 2=KM2, 3=KM3/INS, 4=DipSw)
// =========================
extern uint8 menu_system_mode;
extern uint8 Km13;
extern uint8 KM2_edit_active;   // KM2 GPS路点编辑模式

/* KM2 子模式: 菜单模式下替代拨码开关 */
#define KM2_SUBMODE_OFF       0   // 不在KM2模式
#define KM2_SUBMODE_RECORD    1   // 录点模式
#define KM2_SUBMODE_RUN       2   // 运行模式 (语音+执行)
#define KM2_SUBMODE_CORRECT   3   // GPS修正模式
extern uint8 km2_submode;

// 模式任务函数
void menu_handle_long_press_exit(void);
void dip_switch_mode_task(void);

// KM1/KM2 菜单动作函数声明
void KM1_SaveFlash(void);
void KM1_LoadFlash(void);
void KM1_Run_INS(void);
void KM1_Record(void);
void KM1_EraseFlash(void);
void KM1_ShowPath(void);
void KM1_Run(void);
void KM1_CorrectGPS(void);
void KM1_EditPoints(void);
void KM1_SpeedSet(void);

void KM2_Record(void);
void KM2_SaveFlash(void);
void KM2_LoadFlash(void);
void KM2_ShowPath(void);
void KM2_Run(void);
void KM2_CorrectGPS(void);
void KM2_EditPoints(void);
void KM2_LightTest(void);
void KM2_VoiceTest(void);
void KM2_SpeedSet(void);
void KM2_ActionTest(void);
void KM2_NavMode(void);
void KM2_EditSequence(void);

void dip_switch_mode_enter(void);


#endif
