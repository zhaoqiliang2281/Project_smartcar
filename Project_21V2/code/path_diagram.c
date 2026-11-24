/*
 * path_diagram.c
 *
 *  Created on: 2025年6月9日
 *      Author: liuliangfang
 */

#include "zf_common_headfile.h"

int8 KM_Select_flag = 1;   // 目标选择标志位


void path_image(void)
{
   if(key1_flag == 1)   // 查看路径图(目标一)
   {
       key1_flag = 0;
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
        };
        user_gps_display_init(IPS200_TYPE_SPI, 10, 10, 220, 300);
        user_gps_transition(gps_point_test, sizeof(gps_point_test) / sizeof(gps_point_test[0]));
        user_gps_display(RGB565_PURPLE);

       Buzzer_check(50);
   }

    if(key2_flag == 1)   // 查看路径图(目标二)
    {
        key2_flag = 0;
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
        };
        user_gps_display_init(IPS200_TYPE_SPI, 10, 10, 220, 300);
        user_gps_transition(gps_point_test, sizeof(gps_point_test) / sizeof(gps_point_test[0]));
        user_gps_display(RGB565_YELLOW);
        Buzzer_check(50);
    }

    if(key3_flag == 1)   // 查看路径图
    {
        key3_flag = 0;
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
        };
        user_gps_display_init(IPS200_TYPE_SPI, 10, 10, 220, 300);
        user_gps_transition(gps_point_test, sizeof(gps_point_test) / sizeof(gps_point_test[0]));
        user_gps_display(RGB565_GREEN);

        Buzzer_check(50);
    }
}

//***********************************************************************
// 以下代码原被注释的部分：if(gpio_get_level(sw1) && !gpio_get_level(sw2)) //1开.2关
// {
//     // 修改点位
//     Key_Change_LAT_LON();
// }

int16 GL_NUM = 0;       // 当前点序号（当前选中点索引）
int N = 0;
int Num_flag = 0;       // 方向标志位（用于控制序号递增或递减）

void Key_Change_LAT_LON(void)
{
    /*
     * 目标二：GPS经纬度修改（非阻塞状态机）
     *
     * 按键映射：
     *   key1 - 模式切换：第1次→编辑经度  第2次→编辑纬度  第3次→退出
     *   key2 - 切换选中点位 N（0~15循环，Num_flag控制递增/递减方向）
     *   key3 - 当前坐标 +0.000001°（即+1微度）
     *   key4 - 当前坐标 -0.000001°（即-1微度，带>0防下溢保护）
     *   sw2  - 任意时刻退出编辑
     *
     * 设计要点：
     *   1. 无 while 循环 —— 每帧执行一次，由主循环反复驱动，不阻塞其他任务
     *   2. state 静态变量 —— 记住当前模式，不受 key flag 瞬时值干扰
     *   3. 整数微度运算 —— 直接操作 LAT_data1[N]/LOG_data1[N]（uint32），
     *      避免 double r=0.000001 无法精确表示导致的存盘丢失问题
     *   4. 绘制层次 —— GPS轨迹(底层) → 圆圈(中层) → 文字(顶层)，文字不被覆盖
     */
    if(KM_Select_flag == 2)
    {
        // 静态状态变量：EDIT_IDLE=空闲  EDIT_LOG=编辑经度  EDIT_LAT=编辑纬度
        static enum { EDIT_IDLE, EDIT_LOG, EDIT_LAT } state = EDIT_IDLE;

        // ============================================================
        // 步骤1: key1 循环切换编辑模式
        // ============================================================
        if(key1_flag)
        {
            key1_flag = 0;                          // 消费按键标志
            switch(state)
            {
                case EDIT_IDLE: state = EDIT_LOG;  break;   // 空闲 → 经度编辑
                case EDIT_LOG:  state = EDIT_LAT;  break;   // 经度 → 纬度编辑
                case EDIT_LAT:  state = EDIT_IDLE; return;  // 纬度 → 退出
                default:        state = EDIT_IDLE; return;
            }
            N = 0;  Num_flag = 0;                   // 每次切换模式，复位到第0个点
            ips200_clear();                         // 清屏准备绘制
        }

        if(state == EDIT_IDLE)  return;              // 空闲态：快速退出，不占CPU

        // ============================================================
        // 步骤2: key2 切换当前选中点位 N
        //   Num_flag=0 → N递增(0→15)
        //   Num_flag=1 → N递减(15→0)
        // ============================================================
        if(key2_flag) {
            key2_flag = 0;
            if(Num_flag == 0){N++;ips200_clear();}
            else{N--; ips200_clear();}       // 根据方向标志增减

            if(N == 15) Num_flag = 1;                // 到上限，切换为递减
            if(N == 0)  Num_flag = 0;                // 到下限，切换为递增
        }

        // ============================================================
        // 步骤3: key3/key4 修改当前选中点的坐标
        //   关键：直接操作 LAT_data1[N] 整数（微度），不做浮点 +=
        //   LAT_data1[N] 是 uint32，单位微度（1微度 = 0.000001°）
        //   /1000000.0 与 Read_flash_point() (GPS.c:152) 保持一致
        // ============================================================
        if(state == EDIT_LAT)
        {
            // 纬度模式：直接修改 LAT_data_go
            if(key3_flag) {
                key3_flag=0;
                LAT_data_go[N] += 0.000001;
                ips200_clear();
            }
            if(key4_flag) {
                key4_flag=0;
                if(LAT_data_go[N] > 0.000001) LAT_data_go[N] -= 0.000001;
                ips200_clear();
            }
        }
        else  // EDIT_LOG
        {
            // 经度模式：直接修改 LOG_data_go
            if(key3_flag) {
                key3_flag=0;
                LOG_data_go[N] += 0.000001;
                ips200_clear();
            }
            if(key4_flag) {
                key4_flag=0;
                if(LOG_data_go[N] > 0.000001) LOG_data_go[N] -= 0.000001;
                ips200_clear();
            }
        }

        // ============================================================
        // 步骤4: sw2 退出
        // ============================================================
        if(gpio_get_level(sw2))  { state = EDIT_IDLE; return; }

        // ============================================================
        // 步骤5: 构建 16 点 GPS 数组（在按键修改之后，保证数据最新）
        // ============================================================
        gps_point pts[16];
        for(int i=0; i<16; i++)  { pts[i].lat=LAT_data_go[i]; pts[i].lon=LOG_data_go[i]; }

        // ============================================================
        // 步骤6: 屏幕绘制（由底到顶三层）
        // ============================================================
        //ips200_clear();                              // 清除上一帧内容
        int is_lat      = (state == EDIT_LAT);       // 根据模式选颜色和标签
        uint16 track_clr  = is_lat ? RGB565_BLUE  : RGB565_RED;    // 轨迹色
        uint16 circle_clr = is_lat ? RGB565_YELLOW : RGB565_GREEN;  // 高亮圈色
        char  *lbl        = is_lat ? "LAT" : "LOG";                 // 标签文字
        double show_val    = is_lat ? LAT_data_go[N] : LOG_data_go[N];

        // 第1层: 画 GPS 轨迹连线
        user_gps_display_init(IPS200_TYPE_SPI, 10, 10, 220, 300);
        user_gps_transition(pts, 16);
        user_gps_display(track_clr);

        // 第2层: 画点位圆圈（选中点双圈高亮，普通点小圆点）
        {
            int16 n = user_gps_get_point_count();  uint16 px, py;
            for(int16 i=0; i<n; i++) {
                user_gps_get_screen_point(i, &px, &py);
                if(i==N)  {
                    ips200_draw_circle(px,py,5,circle_clr);         // 内圈半径5
                    ips200_draw_circle(px,py,6,circle_clr);         // 外圈半径6（双圈加粗效果）
                } else {
                    ips200_draw_circle(px,py,2,RGB565_WHITE);       // 普通点半径2
                }
            }
        }

//        printf("%f\n\r",LOG_data_go[N]);//上位机看值
        // 第3层: 画文字信息（最上层，不被轨迹和圆圈覆盖）
        ips200_show_string(0,16*1,"N");              // 点位序号标签
        ips200_show_int  (100,16*1,N,2);             // 点位序号值
        ips200_show_string(0,16*2,lbl);              // LAT 或 LOG 标签
        ips200_show_float(100,16*2,show_val,3,6);    // 当前坐标值（3位整数+6位小数）
        ips200_show_float(8*3,16*3,gnss.latitude,4,6);                    // 实时纬度
        ips200_show_float(8*3,16*4,gnss.longitude,4,6);                   // 实时经度
        ips200_show_float(8*3,16*5,gnss.latitude-LAT_data_go[N],4,6);     // 纬度偏差
        ips200_show_float(8*3,16*6,gnss.longitude-LOG_data_go[N],4,6);    // 经度偏差
    }


}
//************************************************************
