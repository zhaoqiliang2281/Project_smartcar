/*
 * Project2.c
 *
 *  Created on: 2026年3月12日
 *      Author: 耿子卓
 */
#include "zf_common_headfile.h"


uint8 MenDong_Flag = 0;
uint8 MenDong_Flag2 = 0;
uint8 Go_Flag = 1;
uint8 Back_Flag = 0;
int8 first_run=1,IMU_PID=0,finish_flag=1;
double Go_Yaw;
int8 projrect_flag=0;
uint8 Km2_protect_flag=0;
uint8 action_index=0;
uint8 km2_nav_mode = 0;   // 0=GPS返回, 1=INS返回


float S_Sport_yaw[25]={328.96 , 330.67 , 342.58 , 358.26 , 15.49 , 31.88 , 48.56 , 63.81 , 49.37 ,
        24.61 , 359.74 , 334.67 , 308.89 , 295.91 , 309.53 , 326.76 , 343.7 , 1.08 , 18.23 ,
        35.69 , 50.50 , 33.82 , 11.75 , 359.72 , 358.84 };

float SBack_Sport_yaw[25]={148.96 , 150.67 , 162.58 , 178.26 , 195.49 , 211.88 , 228.56 , 243.81 , 229.37 ,
        104.61 , 179.74 , 154.67 , 128.89 , 115.91 , 129.53 , 146.76 , 163.7 , 181.08 , 198.23 ,
        215.69 , 230.50 , 213.82 , 191.75 , 179.72 , 178.84 };


uint8 S_Run_Index = 0;
uint8 S_flag=0;

float S_Yaw_Start = 0;          // 当前段起始Yaw
float S_Yaw_End = 0;            // 当前段结束Yaw
float S_Yaw_Target = 0;         // 目标值目标Yaw
float S_Last_Yaw_Target = 0;    // 上一目标Yaw
int32 S_Last_Encoder_Read=0;
int32 S_Read_Continue_Encoder_Left=0;
int32 S_delta2;

#define S_YAW_MAX_CHANGE   20.0f // 限制Yaw变化最大值

float S_start_yaw;


/************************************************
 *  函数名称: 目标2路径点，包含路径点设置，由System_NSetting_KM2(void)
 *  函数说明:
 *  输入说明:
 *  输   出:
 *  返回值:
 *  更新时间: 2025年6月26日
 *  备注: 包含路径点1路径点，包含路径点2路径点，包含第3段
 ************************************************/
void System_NSetting_KM2(void)
{
    if(!gpio_get_level(sw1) &&  !gpio_get_level(sw2))//1,2关
    {
        ips200_show_string(230, 16*8, "S");
        GPS_array();
        if(key2_flag == 1)
        {
            key2_flag=0;
            Up_flash_point();
            FLASH_Get_KM2();   //将目标存入flash中
            Buzzer_check(50);
        }
        if(key3_flag == 1)
        {
            key3_flag=0;
            FLASH_READ_KM2();   //读取目标(读取flash)
//            GPS_Remote();//gps漂移修正
            Read_flash_point();
            Buzzer_check(50);

        }
    }
    else if(gpio_get_level(sw1) &&  gpio_get_level(sw2))//1,2开
    {
        ips200_show_string(230, 16*8, "X");
        if(key1_flag == 1)
        {
            key1_flag=0;
            next = 1;
            gps_point gps_point_test[]=
            {
                {LAT_data_go[0],LOG_data_go[0]},
                {LAT_data_go[1],LOG_data_go[1]},
                {LAT_data_go[2],LOG_data_go[2]},
                {LAT_data_go[3],LOG_data_go[3]},
                {LAT_data_go[4],LOG_data_go[4]},
                {LAT_data_go[5],LOG_data_go[5]},
                {LAT_data_go[6],LOG_data_go[6]},
                {LAT_data_go[7],LOG_data_go[7]},

                {LAT_data_go[8],LOG_data_go[8]},
                {LAT_data_go[9],LOG_data_go[9]},
                {LAT_data_go[10],LOG_data_go[10]},
                {LAT_data_go[11],LOG_data_go[11]},
                {LAT_data_go[12],LOG_data_go[12]},
                {LAT_data_go[13],LOG_data_go[13]},//索引14个点
                {LAT_data_go[14],LOG_data_go[14]},//索引15个点
                {LAT_data_go[15],LOG_data_go[15]},//索引16个点
            };
            user_gps_display_init(IPS200_TYPE,10,10,220,300);
            user_gps_transition(gps_point_test,sizeof(gps_point_test) / sizeof(gps_point_test[0]));
            user_gps_display(RGB565_PURPLE);

            Buzzer_check(50);
        }
        if(key2_flag == 1)
        {
            key2_flag=0;
            Buzzer_SetTimer(50);
            Km2_protect_flag = 1;
            dispatch_car = 1;
        }

        if(key3_flag == 1)
        {
            key3_flag=0;
            Up_flash_point_2();
            FLASH_Get_KM2();   //将目标再次放大存入flash中

            Buzzer_check(50);
        }
        if(key4_flag==1)
        {
                key4_flag=0;
                Buzzer_SetTimer(50);

        }

    }
    else if(gpio_get_level(sw1) &&  !gpio_get_level(sw2))//1开,2关
    {

        KM2_WaypointEditor();//GPS路径修改

    }
    else if(!gpio_get_level(sw1) &&  gpio_get_level(sw2))//1关,2开
    {
        if(key3_flag == 1)
        {
            key3_flag=0;
            Buzzer_check(50);
            LAT_error=gnss.latitude-LAT_data_go[0];
            LOG_error=gnss.longitude-LOG_data_go[0];
            for(int i=0;i<16;i++)
            {
                LAT_data_go[i]+=LAT_error;
                LOG_data_go[i]+=LOG_error;
            }
        }

        if(key4_flag == 1)
        {
            key4_flag=0;
            Buzzer_check(50);
/*          data_store[0x09,0x05,0x16,0x1C,0x26,0x0D];*/
            data_store[0]=0x09;
            data_store[1]=0x05;
            data_store[2]=0x16;
            data_store[3]=0x1C;
            data_store[4]=0x26;
            data_store[5]=0x0D;

            Sport_sum_flag=1;
            Flash_sum_flag=2;
            Voice_sum_flag=1;
            data_store_count=6;
            CI1302_get_command_name(data_store[0]);
            CI1302_get_command_name(data_store[1]);
            CI1302_get_command_name(data_store[2]);
            CI1302_get_command_name(data_store[3]);
            CI1302_get_command_name(data_store[4]);
            CI1302_get_command_name(data_store[5]);


        }


    }


}

/************************************************
 *  函数名称: 目标执行
 *  函数说明:
 *  输入说明: Task在中断中去执行时执行一段路径
 *            Task2中断断开时执行另一段路径
 *  输   出:
 *  返回值:
 *  更新时间: 2025年6月26日
 *  备注: 包含路径点1路径点，包含路径点2路径点，包含第3段
 ************************************************/

/************************************************
 *  函数名称: KM2_StateMachine (公共状态机, menu和dip_switch共用)
 *  功能说明：统一 KM2 灯光->导航->语音运动 三阶段执行
 *  参数说明：无
 *  修改时间：2026年7月9日
 *  备   注：避免 menu 和 dip_switch 两套状态机不同步
 ************************************************/
void KM2_NavMode(void)
{
    km2_nav_mode = !km2_nav_mode;
    menu_set_status(km2_nav_mode ? "KM2 Return: INS" : "KM2 Return: GPS");
    Buzzer_check(50);
}

void KM2_StateMachine(void)
{
    /* 计算循迹数据占用的字节数: data_store中连续2字节为Track指令(0x15~0x1E) */
    static uint8 track_bytes = 2;  // 默认2字节(预设序列布局)

    switch(Km2_protect_flag)
    {
        case 0:   /* 空闲 */
            ips200_show_string(150, 16*8, "IDLE");
            break;
        case 1:   /* 灯光阶段 */
            ips200_show_string(150, 16*8, "1/3 LIGHT");
            if(Flash_sum_flag == 0 || Flash_choise(data_store, Flash_sum_flag, 0) == 0)
            {
                /* 检测是否有循迹指令,提前计算track_bytes供后续阶段使用 */
                uint8 t0 = (Flash_sum_flag < data_store_count) ? data_store[Flash_sum_flag] : 0;
                track_bytes = ((t0 >= 0x15 && t0 <= 0x1E) ? 2 : 0);
                Km2_protect_flag = 2;
                system_delay_ms(500);
            }
            break;
        case 2:   /* 鸣笛/语音阶段 (原case3提前) */
            ips200_show_string(150, 16*8, "2/3 VOICE");
            if(Voice_sum_flag == 0 ||
               Voice_choise(data_store, Voice_sum_flag, Flash_sum_flag + track_bytes + Sport_sum_flag) == 0)
            {
                Km2_protect_flag = 3;
                system_delay_ms(500);
            }
            break;
        case 3:   /* GPS导航阶段(仅前进,运动完成后停车) */
            ips200_show_string(150, 16*8, "3/3 TRACK");
            if(dispatch_car == 1 && next == 1 && Transform == 1)
            {
                Back_Motor_SetPwm(300, 300);
            }
            if(track_bytes == 0 ||
               Track2_Km2(data_store[Flash_sum_flag], data_store[Flash_sum_flag + 1]) == 0)
            {
                Km2_protect_flag = 0;
                system_delay_ms(500);
            }
            break;
    }
}

/************************************************
 *  函数名称: KM2_KeyHandler (菜单模式按键处理, 不依赖拨码开关)
 *  功能说明：根据 km2_submode 分发 KEY1~4 功能
 *            替代 System_NSetting_KM2 中的 sw1/sw2 分支逻辑
 ************************************************/
void KM2_KeyHandler(void)
{
    switch(km2_submode)
    {
        case KM2_SUBMODE_RECORD:   /* 录点模式 */
            ips200_show_string(230, 16*8, "M");
            GPS_array();          /* KEY1: 记录GPS点 */
            if(key2_flag == 1)   /* KEY2: 保存到Flash */
            {
                key2_flag = 0;
                Up_flash_point();
                FLASH_Get_KM2();
                Buzzer_check(50);
                menu_set_status("Saved to Flash");
            }
            if(key3_flag == 1)   /* KEY3: 撤销上一个点 */
            {
                key3_flag = 0;
                gps_index_undo();
                ips200_show_string(0, 16*6, "Rec:"); ips200_show_int(30, 16*6, get_gps_index(), 2);
                ips200_show_string(55, 16*6, "/16");
                Buzzer_check(50);
            }
            break;

        case KM2_SUBMODE_RUN:      /* 运行模式 */
            ips200_show_string(230, 16*8, "R");
            if(key1_flag == 1)   /* KEY1: 显示GPS路径 */
            {
                key1_flag = 0;
                next = 1;
                gps_point gps_point_test[] = {
                    {LAT_data_go[0],LOG_data_go[0]},  {LAT_data_go[1],LOG_data_go[1]},
                    {LAT_data_go[2],LOG_data_go[2]},  {LAT_data_go[3],LOG_data_go[3]},
                    {LAT_data_go[4],LOG_data_go[4]},  {LAT_data_go[5],LOG_data_go[5]},
                    {LAT_data_go[6],LOG_data_go[6]},  {LAT_data_go[7],LOG_data_go[7]},
                    {LAT_data_go[8],LOG_data_go[8]},  {LAT_data_go[9],LOG_data_go[9]},
                    {LAT_data_go[10],LOG_data_go[10]},{LAT_data_go[11],LOG_data_go[11]},
                    {LAT_data_go[12],LOG_data_go[12]},{LAT_data_go[13],LOG_data_go[13]},
                    {LAT_data_go[14],LOG_data_go[14]},{LAT_data_go[15],LOG_data_go[15]},
                };
                user_gps_display_init(IPS200_TYPE, 10, 10, 220, 300);
                user_gps_transition(gps_point_test, sizeof(gps_point_test)/sizeof(gps_point_test[0]));
                user_gps_display(RGB565_PURPLE);
                Buzzer_check(50);
            }
            if(key2_flag == 1)   /* KEY2: 触发发车 */
            {
                key2_flag = 0;
                Buzzer_SetTimer(50);
                Km2_protect_flag = 1;
                dispatch_car = 1;
            }
            if(key3_flag == 1)   /* KEY3: 撤销上一条语音指令 */
            {
                key3_flag = 0;
                if(data_store_count > 0)
                {
                    uint8 last = data_store[data_store_count - 1];
                    if(last >= 0x04 && last <= 0x0B)       Flash_sum_flag--;
                    else if(last >= 0x1F && last <= 0x26)  Sport_sum_flag--;
                    else if(last >= 0x0C && last <= 0x14)  Voice_sum_flag--;
                    data_store[data_store_count - 1] = 0;
                    data_store_count--;
                    /* 清除屏幕上对应的中文显示行 */
                    if(Chinese_task_display > 0)
                    {
                        uint16 y0 = (uint16)(16 * Chinese_task_display);
                        uint16 y1 = (uint16)(16 * Chinese_task_display + 15);
                        for(uint16 y = y0; y <= y1; y++)
                            ips200_draw_line(0, y, 239, y, RGB565_BLACK);
                        Chinese_task_display--;
                    }
                    menu_set_status("Last cmd removed");
                    Buzzer_check(50);
                }
            }
            if(key4_flag == 1)   /* KEY4: GPS整体偏移修正 */
            {
                key4_flag = 0;
                Buzzer_check(50);
                LAT_error = gnss.latitude  - LAT_data_go[0];
                LOG_error = gnss.longitude - LOG_data_go[0];
                for(int i = 0; i < 16; i++)
                {
                    LAT_data_go[i] += LAT_error;
                    LOG_data_go[i] += LOG_error;
                }
                km2_de2xy();
                menu_set_status("GPS corrected to current pos");
            }
            break;

        case KM2_SUBMODE_CORRECT:  /* GPS修正模式 */
            ips200_show_string(230, 16*8, "C");
            if(key3_flag == 1)   /* KEY3: GPS整体偏移修正 */
            {
                key3_flag = 0;
                Buzzer_check(50);
                LAT_error = gnss.latitude - LAT_data_go[0];
                LOG_error = gnss.longitude - LOG_data_go[0];
                for(int i = 0; i < 16; i++)
                {
                    LAT_data_go[i] += LAT_error;
                    LOG_data_go[i] += LOG_error;
                }
            }
            if(key4_flag == 1)   /* KEY4: 写入预设指令序列 */
            {
                key4_flag = 0;
                Buzzer_check(50);
                data_store[0]=0x09; data_store[1]=0x05; data_store[2]=0x16;
                data_store[3]=0x1C; data_store[4]=0x26; data_store[5]=0x0D;
                Sport_sum_flag=3; Flash_sum_flag=2; Voice_sum_flag=1;
                for(int i = 0; i < 6; i++)
                    CI1302_get_command_name(data_store[i]);
            }
            break;

        default:
            break;
    }
}

uint8 Track2_Km2(uint8 Task,uint8 Task2)
{

    static uint8 sport_finish=0;

    /* GPS跳变检测 */
    static double last_lat = 0, last_lon = 0;
    static uint8 gps_ok = 1;
    if(last_lat == 0) { last_lat = gnss.latitude; last_lon = gnss.longitude; }
    if(gnss.state == 1)
    {
        double dlat = gnss.latitude  - last_lat;
        double dlon = gnss.longitude - last_lon;
        double jump = sqrt(dlat*dlat + dlon*dlon) * 111000.0;  /* deg -> m */
        if(jump > 3.0) { gps_ok = 0; }  /* GPS跳变，标记不可靠 */
        else if(jump > 0.1) { gps_ok = 1; last_lat = gnss.latitude; last_lon = gnss.longitude; }
    }
    ips200_show_string(150, 16*9, gps_ok ? "GPS OK" : "GPS ??");

    get_point_distance();       //将实时位置存入home_run[L]中对比实时距离
    GPS_Two_point_error();      //计算方位角度
    GPS_Two_point_Distance();   //要使用距离作为条件，使用距离标志开启PID

    /* INS导航: 全路段, 基于相对偏移 */
    static float ins_fwd_ref_x, ins_fwd_ref_y;  // 前进参考点(路点0)
    static float ins_ret_ref_x, ins_ret_ref_y;  // 返回参考点(路点12)
    static uint8  ins_fwd_init = 0;
    static uint8  ins_ret_init = 0;
    static uint8  last_back = 0;

    uint8 ins_valid = (fabs(KM2_INS_X[0]) > 0.01f || fabs(KM2_INS_Y[0]) > 0.01f);

    /* 重置: 发车时清零 */
    if(next <= 1 && Go_Flag == 1) { ins_fwd_init = 0; ins_ret_init = 0; last_back = 0; }

    if(km2_nav_mode == 1 && ins_valid && next > 0 && next <= 16)
    {
        if(Go_Flag == 1)
        {
            /* 前进段: 以路点0为参考 */
            if(!ins_fwd_init) {
                ins_fwd_ref_x = INS_X; ins_fwd_ref_y = INS_Y;
                ins_fwd_init = 1;
            }
            float dx = KM2_INS_X[next-1] - KM2_INS_X[0];
            float dy = KM2_INS_Y[next-1] - KM2_INS_Y[0];
            float tx = ins_fwd_ref_x + dx;
            float ty = ins_fwd_ref_y + dy;
            azimuth = Get_Target_Angle(INS_X, INS_Y, tx, ty);
            home_run[next] = Get_Two_Point_Distance(INS_X, INS_Y, tx, ty);
        }
        else if(Back_Flag == 1)
        {
            /* 返回段: 以动作区(路点12)为参考 */
            if(last_back == 0 && Back_Flag == 1) ins_ret_init = 0;
            last_back = Back_Flag;
            if(!ins_ret_init) {
                ins_ret_ref_x = INS_X; ins_ret_ref_y = INS_Y;
                ins_ret_init = 1;
            }
            float dx = KM2_INS_X[next-1] - KM2_INS_X[12];
            float dy = KM2_INS_Y[next-1] - KM2_INS_Y[12];
            float tx = ins_ret_ref_x + dx;
            float ty = ins_ret_ref_y + dy;
            azimuth = Get_Target_Angle(INS_X, INS_Y, tx, ty);
            home_run[next] = Get_Two_Point_Distance(INS_X, INS_Y, tx, ty);
        }
    }

    ips200_show_int(0, 100, next, 2);//显示实时点序号

    switch(Task)
    {
        case 0x15: //路径1的起点
            MenDong_Flag=1;
            break;
        case 0x16: //路径1
            MenDong_Flag=3;
            break;
        case 0x17: //路径2
            MenDong_Flag=5;
            break;
        case 0x18: //路径3
            MenDong_Flag=7;
            break;
        case 0x19: //路径3右侧
            MenDong_Flag=9;
            break;
    }
    switch(Task2)
    {
        case 0x1A: //路径1的起点
            MenDong_Flag2=1;
            break;
        case 0x1B: //路径1
            MenDong_Flag2=3;
            break;
        case 0x1C: //路径2
            MenDong_Flag2=5;
            break;
        case 0x1D: //路径3
            MenDong_Flag2=7;
            break;
        case 0x1E: //路径3右侧
            MenDong_Flag2=9;
            break;
    }

    if(home_run[next] < need_distance  && home_run[0]!=0 && next==1 && Go_Flag==1)
       {
           next=MenDong_Flag+1;

           Back_Motor_PID_Start_flag = 1; //为1时打开位置PID

           Back_Motor_Set_Speed = Speed_KM2_Buf[0];//前进速度
           Buzzer_SetTimer(50);
       }
       if(home_run[next] < need_distance  && next == MenDong_Flag+1 && Go_Flag==1)
       {
           next = MenDong_Flag+1+1;
           Back_Motor_Set_Speed = Speed_KM2_Buf[1];//前进速度
           Buzzer_SetTimer(50);
       }
        if(home_run[next] <need_distance  && next == MenDong_Flag+1+1 && Go_Flag==1)
        {
            next = 11+1;
            Back_Motor_Set_Speed = Speed_KM2_Buf[2];//前进速度
            Buzzer_SetTimer(50);
        }
        if(home_run[next] <need_distance  && next == 11+1 && Go_Flag==1)
        {
            next = 12+1;
            Back_Motor_Set_Speed = Speed_KM2_Buf[2];//前进速度
            Buzzer_SetTimer(50);
        }
        if(home_run[next] <need_distance  && next == 12+1 && Go_Flag==1)
        {
            next = MenDong_Flag2+1+1;
            Go_Flag=0;
            delay_ScanTemp = 0;//关闭定时器标志位
            dispatch_car = 0;//关闭转向标志位
            Back_Motor_Set_Speed = Speed_KM2_Buf[7];//前进速度

            Back_Motor_PID_Start_flag = 0; //关闭位置PID后速度归零
            Buzzer_SetTimer(1000);
        }
          //转向PID选择
        if(dispatch_car == 1)
        {
          Control_straight(1);  // KM2统一使用前进PID,与KM1/KM3一致
        }
        if(IMU_PID == 1)
        {
            IMU_Control(Go_Yaw-Yaw);
        }



          // 电机速度 (差速转向)
        if((Go_Flag==1 || Back_Flag==1) && next > 1 )
        {
          Back_Motor_SetPwm_Diff(Back_Motor_Set_Speed, Error);
        }
          //运动写进定时中断后清除
        if(Go_Flag==0 && Back_Flag==0)
        {

            sport_finish=Sport_choise(data_store,Sport_sum_flag,Flash_sum_flag+2);
            if(!sport_finish)
            {
                // 运动完成,直接停车,不再返回
                Back_Motor_SetPwm(0,0);
                Back_Motor_PID_Start_flag = 0;
                dispatch_car = 0;
                Buzzer_SetTimer(1000);
                return 0;
            }
//

        }
          //***********************


        // ==== 以下返回导航代码已废弃(运动完成后直接停车) ====
        if(0 && home_run[next] < need_distance  && next == MenDong_Flag2+1+1 && Back_Flag==1)
        {
            next = MenDong_Flag2+1;
            Back_Motor_Set_Speed = Speed_KM2_Buf[next-1];//前进速度
            Buzzer_SetTimer(50);
        }
        if(0 && home_run[next] < need_distance  && next == MenDong_Flag2+1 && Back_Flag==1)
        {
           next = 13+1;
           Back_Motor_Set_Speed = Speed_KM2_Buf[next-1];//前进速度
           Buzzer_SetTimer(50);
        }
        if(0 && home_run[next] < need_distance  && next == 13+1 && Back_Flag==1)
       {
          next = 14+1;
          Back_Motor_Set_Speed = Speed_KM2_Buf[next-1];//前进速度
          Buzzer_SetTimer(50);
       }
        if(0 && home_run[next] <0.8  && next == 14+1 && Back_Flag==1)
        {
            next = 15+1;
            delay_ScanTemp = 0;//关闭定时器标志位
            dispatch_car = 0;//关闭转向标志位
            Back_Motor_Set_Speed = 0;//速度归零
            Back_Motor_SetPwm(0,0);
            Back_Motor_PID_Start_flag = 0; //关闭位置PID后速度归零
            Buzzer_SetTimer(1000);
            return 0;
        }

        return 1;
}


/************************************************
 *  函数名称: 角度偏差pid
 *  函数说明:
 *  输入说明: Expectation_Yaw为当前期望角度
 *  输   出:
 *  返回值:
 *  更新时间: 2025年6月26日
 *  备注: 包含路径点1路径点，包含路径点2路径点，包含第3段
 ************************************************/
void IMU_Control(double Expectation_Yaw)
{
    if(Expectation_Yaw<-180)
        Expectation_Yaw+=360;
    Front_Dir_PID_Control(0, Expectation_Yaw, &pid_gps);
}


int16 Get_Yaw_Error(int16 target, int16 current)
{
    int16 error = target - current;

    if(error > 180)
        error -= 360;

    if(error < -180)
        error += 360;

    return error;
}

uint8 Go_straight(void)//直行前进10m
{

    static int8 my_first = 1;    static int32 first_encoder = 0;
    static int32 Moving_Distance=0;//行驶距离
    if(my_first==1)
    {
        Go_Yaw = Yaw;
        Angle_Target_Position = 0;
        first_encoder=((Read_Encoder_Aggregate_Value(TIM5_ENCODER)+Read_Encoder_Aggregate_Value(TIM2_ENCODER))/2);
        my_first=0;
    }

    IMU_PID=1;
    Back_Motor_SetPwm(300,300);
    Moving_Distance = ((Read_Encoder_Aggregate_Value(TIM5_ENCODER)+Read_Encoder_Aggregate_Value(TIM2_ENCODER))/2) - first_encoder;
    if(Moving_Distance>13150)//十米
    {
        Buzzer_check(50);
        Back_Motor_SetPwm(0,0);
        IMU_PID=0;
        my_first=1;
        system_delay_ms(500);
        return 0;
    }
    return 1;
}

uint8 Back_straight(void)//直行后退10m
{

    static int8 my_first = 1;    static int32 first_encoder = 0;
    static int32 Moving_Distance=0;//行驶距离
    if(my_first==1)
    {
        Go_Yaw = Yaw;
        Angle_Target_Position = 0;
        first_encoder=((Read_Encoder_Aggregate_Value(TIM5_ENCODER)+Read_Encoder_Aggregate_Value(TIM2_ENCODER))/2);
        my_first=0;
    }
    IMU_PID=1;
    Back_Motor_SetPwm(-300,-300);
    Moving_Distance = ((Read_Encoder_Aggregate_Value(TIM5_ENCODER)+Read_Encoder_Aggregate_Value(TIM2_ENCODER))/2) - first_encoder;
    if(Moving_Distance<-13150)
    {

    Buzzer_check(50);
        Back_Motor_SetPwm(0,0);
        IMU_PID=0;
        my_first=1;
        system_delay_ms(500);
        return 0;
    }
    return 1;
}

uint8 Left_straight(void)//左转
{
    static int8 my_first = 1;
    if(my_first)
    {
        Go_Yaw = (Yaw - 90 < 0) ? ((Yaw - 90) + 360) : (Yaw - 90);
        my_first=0;
        Angle_Target_Position = -900;
    }
    Back_Motor_SetPwm(300,300);
    if(abs(Get_Yaw_Error(Yaw,Go_Yaw)<=2))
    {

    static int8 my_first = 1;        Buzzer_check(50);
        Back_Motor_SetPwm(0,0);
        IMU_PID=0;
        Angle_Target_Position = 0;
        my_first=1;
        system_delay_ms(500);
        return 0;
    }
    return 1;
}

uint8 Right_straight(void)//右转
{
    static int8 my_first = 1;
    if(my_first)
    {
        Go_Yaw = (Yaw + 90 >= 360) ? (Yaw + 90 - 360) : (Yaw + 90);
        my_first=0;
        Angle_Target_Position = 900;
    }
    Back_Motor_SetPwm(300,300);
    if(abs(Get_Yaw_Error(Yaw,Go_Yaw)) <= 2)
    {

    static int8 my_first = 1;        Back_Motor_SetPwm(0,0);
        IMU_PID=0;
        Buzzer_check(50);
        Angle_Target_Position = 0;
        my_first=1;
        system_delay_ms(500);
        return 0;
    }
    return 1;
}

uint8 Left_circlet(void)//左转一圈
{

        static int8 Yaw_Flag=1;
        static int8 my_first = 1;
        if(my_first)
        {
            Go_Yaw=Yaw;
            my_first=0;
            Yaw_Flag = 1;
            Angle_Target_Position = -900;
            system_delay_ms(500);
        }
        if((Yaw<=Go_Yaw-2 || Yaw>=Go_Yaw+2) && Yaw_Flag==1)
        {
            Yaw_Flag=0;
        }
        Back_Motor_SetPwm(300,300);
        if((Yaw>=Go_Yaw-2 && Yaw<=Go_Yaw+2) && Yaw_Flag == 0)
        {

    static int8 my_first = 1;            Back_Motor_SetPwm(0,0);
            Buzzer_check(50);
            Angle_Target_Position = 0;
            my_first=1;
            system_delay_ms(500);
            return 0;
        }
        return 1;


}

uint8 Right_circlet(void)//右转一圈
{
    static int8 Yaw_Flag=1;
    static int8 my_first = 1;
    if(my_first)
    {
        my_first=0;
        Go_Yaw=Yaw;
        Yaw_Flag = 1;
        Angle_Target_Position = 900;
        system_delay_ms(500);
    }
    if((Yaw<=Go_Yaw-2 || Yaw>=Go_Yaw+2) && Yaw_Flag==1)
    {
        Yaw_Flag=0;
    }
    Back_Motor_SetPwm(300,300);
    if((Yaw>=Go_Yaw-2 && Yaw<=Go_Yaw+2) && Yaw_Flag == 0)
    {
        Buzzer_check(50);
        Back_Motor_SetPwm(0,0);
        Angle_Target_Position = 0;
        first_run=1;
        system_delay_ms(500);
        return 0;
    }
    return 1;
}

uint8 Go_S(void)
{

    float t;
    float yaw_diff;
    float yaw_change;

    //==================== 初始化 ====================//

    if(S_flag == 0)
    {
        S_start_yaw = Yaw;  // 先获取当前实时角度作为偏转基准

        S_Yaw_Start = S_Sport_yaw[0] + S_start_yaw;
        // 终点Yaw也要加偏转
        S_Yaw_End = S_Sport_yaw[1] + S_start_yaw;

        // 归一化到0~360
        if(S_Yaw_Start >= 360.0f) S_Yaw_Start -= 360.0f;
        if(S_Yaw_Start < 0.0f) S_Yaw_Start += 360.0f;
        if(S_Yaw_End >= 360.0f) S_Yaw_End -= 360.0f;
        if(S_Yaw_End < 0.0f) S_Yaw_End += 360.0f;

        S_Yaw_Target = S_Yaw_Start;

        S_Last_Yaw_Target = S_Yaw_Start;

        S_Run_Index = 1;
        S_Last_Encoder_Read = ((Read_Encoder_Aggregate_Value(TIM5_ENCODER)+Read_Encoder_Aggregate_Value(TIM2_ENCODER))/2);
        S_flag = 1;

    }

    //================================================//

    S_Read_Continue_Encoder_Left = ((Read_Encoder_Aggregate_Value(TIM5_ENCODER)+Read_Encoder_Aggregate_Value(TIM2_ENCODER))/2);
    // 当前累计行驶距离
    S_delta2 = S_Read_Continue_Encoder_Left - S_Last_Encoder_Read;

    //==================== 赋值 ====================//
    // 当前段进度
    t = (float)abs(S_delta2) / (float)Nag_Set_mileage;

    if(t > 1.0f)
    {
        t = 1.0f;
    }

    // 计算段内插值
    yaw_diff = angle_diff(S_Yaw_End, S_Yaw_Start);

    // 线性插值目标
    S_Yaw_Target = S_Yaw_Start + yaw_diff * t;

    // 归一化0~360
    if(S_Yaw_Target >= 360.0f)
    {
        S_Yaw_Target -= 360.0f;
    }

    if(S_Yaw_Target < 0.0f)
    {
        S_Yaw_Target += 360.0f;
    }

    //==================== 限幅处理 ====================//

    // 注意：
    // 使用 angle_diff
    // 直接限幅
    yaw_change = angle_diff(S_Yaw_Target, S_Last_Yaw_Target);
    // 限制每帧变化量

    if(yaw_change > S_YAW_MAX_CHANGE)
    {
        yaw_change = S_YAW_MAX_CHANGE;
    }

    if(yaw_change < -S_YAW_MAX_CHANGE)
    {
        yaw_change = -S_YAW_MAX_CHANGE;
    }
    // 更新实际Yaw_Target

    S_Yaw_Target = S_Last_Yaw_Target + yaw_change;
    // 归一化0~360

    if(S_Yaw_Target >= 360.0f)
    {
        S_Yaw_Target -= 360.0f;
    }

    if(S_Yaw_Target < 0.0f)
    {
        S_Yaw_Target += 360.0f;
    }

    // 更新
    S_Last_Yaw_Target = S_Yaw_Target;

    // 前进
    Angle_Target_Position = -GPS_IMU_PID_Start(Open, angle_diff(S_Yaw_Target, Yaw), 0, 1);
    Back_Motor_SetPwm(200, 200);

    //==================== 切换到下一段 ====================//

    if(abs(S_delta2) >= Nag_Set_mileage)
    {
        S_Run_Index++;
        // 停止
        if(S_Run_Index >= 25)
        {
            Back_Motor_SetPwm(0,0);
            Angle_Target_Position = 0;
            S_flag = 0;
            system_delay_ms(500);
            return 0;
        }

          // 当前终点变为下一段起点
        S_Yaw_Start = S_Yaw_End;

        // 读取下一段终点（也要加偏转）
        S_Yaw_End = S_Sport_yaw[S_Run_Index] + S_start_yaw;

        // 归一化
        if(S_Yaw_End >= 360.0f) S_Yaw_End -= 360.0f;
        if(S_Yaw_End < 0.0f) S_Yaw_End += 360.0f;

        S_Last_Encoder_Read = S_Read_Continue_Encoder_Left;
    }
    return 1;
}

uint8 Back_S(void)
{
    float t;
    float yaw_diff;
    float yaw_change;

    //==================== 初始化 ====================//

    if(S_flag == 0)
    {
        S_start_yaw = Yaw;  // 先获取当前实时角度作为偏转基准

        S_Yaw_Start = SBack_Sport_yaw[24] + S_start_yaw;
        // 终点Yaw也要加偏转
        S_Yaw_End = SBack_Sport_yaw[23] + S_start_yaw;

        // 归一化到0~360
        if(S_Yaw_Start >= 360.0f) S_Yaw_Start -= 360.0f;
        if(S_Yaw_Start < 0.0f) S_Yaw_Start += 360.0f;
        if(S_Yaw_End >= 360.0f) S_Yaw_End -= 360.0f;
        if(S_Yaw_End < 0.0f) S_Yaw_End += 360.0f;

        S_Yaw_Target = S_Yaw_Start;

        S_Last_Yaw_Target = S_Yaw_Start;

        S_Run_Index = 23;
        S_Last_Encoder_Read = ((Read_Encoder_Aggregate_Value(TIM5_ENCODER)+Read_Encoder_Aggregate_Value(TIM2_ENCODER))/2);
        S_flag = 1;

    }

    //================================================//

    S_Read_Continue_Encoder_Left = ((Read_Encoder_Aggregate_Value(TIM5_ENCODER)+Read_Encoder_Aggregate_Value(TIM2_ENCODER))/2);
    // 当前累计行驶距离
    S_delta2 = S_Read_Continue_Encoder_Left - S_Last_Encoder_Read;

    //==================== 赋值 ====================//
    // 当前段进度
    t = (float)abs(S_delta2) / (float)Nag_Set_mileage;

    if(t > 1.0f)
    {
        t = 1.0f;
    }

    // 计算段内插值
    yaw_diff = angle_diff(S_Yaw_End, S_Yaw_Start);

    // 线性插值目标
    S_Yaw_Target = S_Yaw_Start + yaw_diff * t;

    // 归一化0~360
    if(S_Yaw_Target >= 360.0f)
    {
        S_Yaw_Target -= 360.0f;
    }

    if(S_Yaw_Target < 0.0f)
    {
        S_Yaw_Target += 360.0f;
    }

    //==================== 限幅处理 ====================//

    // 注意：
    // 使用 angle_diff
    // 直接限幅
    yaw_change = angle_diff(S_Yaw_Target, S_Last_Yaw_Target);
    // 限制每帧变化量

    if(yaw_change > S_YAW_MAX_CHANGE)
    {
        yaw_change = S_YAW_MAX_CHANGE;
    }

    if(yaw_change < -S_YAW_MAX_CHANGE)
    {
        yaw_change = -S_YAW_MAX_CHANGE;
    }
    // 更新实际Yaw_Target

    S_Yaw_Target = S_Last_Yaw_Target + yaw_change;
    // 归一化0~360

    if(S_Yaw_Target >= 360.0f)
    {
        S_Yaw_Target -= 360.0f;
    }

    if(S_Yaw_Target < 0.0f)
    {
        S_Yaw_Target += 360.0f;
    }

    // 更新
    S_Last_Yaw_Target = S_Yaw_Target;

    // 前进
    Angle_Target_Position = -GPS_IMU_PID_Start(Open, angle_diff(S_Yaw_Target, Yaw), 0, -1);
    Back_Motor_SetPwm(-200, -200);

    //==================== 切换到下一段 ====================//

    if(abs(S_delta2) >= Nag_Set_mileage)
    {
        S_Run_Index--;
        // 停止
        if(S_Run_Index <= 0)
        {
            Back_Motor_SetPwm(0,0);
            Angle_Target_Position = 0;
            S_flag = 0;
            system_delay_ms(500);
            return 0;
        }

          // 当前终点变为下一段起点
        S_Yaw_Start = S_Yaw_End;

        // 读取下一段终点（也要加偏转）
        S_Yaw_End = SBack_Sport_yaw[S_Run_Index] + S_start_yaw;

        // 归一化
        if(S_Yaw_End >= 360.0f) S_Yaw_End -= 360.0f;
        if(S_Yaw_End < 0.0f) S_Yaw_End += 360.0f;

        S_Last_Encoder_Read = S_Read_Continue_Encoder_Left;
    }
    return 1;
}

/************************************************
 *  函数名称: 运动调度
 *  函数说明:
 *  输入说明: sport_sum为要执行的运动数目0-8个
 *         sprot_num[]:要执行的运动标志位
 *         first_num为运动起始位置
 *  输   出:
 *  返回值:
 *  更新时间: 2026年5月21日
 *  备   注:
 ************************************************/
uint8 Sport_choise(uint8 sprot_num[],uint8 sport_sum,uint8 first_num)
{
    static uint8 sport_flag = 0;
    static uint8 sport_continue;
    static uint8 is_initialized = 0;  // 判断是否已经初始化
    if (!is_initialized) {
        sport_flag = first_num;  // 第一次调用时初始化
        is_initialized = 1;
    }

    if(sport_flag < (first_num+sport_sum))
    {
        switch(sprot_num[sport_flag])
        {
            case 0x1F:             //直行10m
                sport_continue=Go_straight();
                if(sport_continue ==0)
                    sport_flag++;
                break;

            case 0x20:             //后退10m
                sport_continue=Back_straight();
                if(sport_continue ==0)
                    sport_flag++;
                break;

            case 0x25:             //左转
                sport_continue=Left_straight();
                if(sport_continue ==0)
                    sport_flag++;
                break;

            case 0x26:             //右转
                sport_continue=Right_straight();
                if(sport_continue ==0)
                    sport_flag++;
                break;

            case 0x23:             //左转一圈
                sport_continue=Left_circlet();
                if(sport_continue ==0)
                    sport_flag++;
                break;

            case 0x24:             //右转一圈
                sport_continue=Right_circlet();
                if(sport_continue ==0)
                    sport_flag++;
                break;

            case 0x21:             //执行前路径
                sport_continue=Go_S();
                if(sport_continue ==0)
                    sport_flag++;
                break;

            case 0x22:             //执行后路径
                sport_continue=Back_S();
                if(sport_continue ==0)
                    sport_flag++;
                break;
        }
        return 1;
    }
    else
    {
        return 0;
    }
}


/************************************************
 *  函数名称: 灯光调度
 *  函数说明:
 *  输入说明: Flash_sum为要执行的灯光数目0-8个
 *         Flash_num[]:要执行的灯光标志位
 *         first_num为灯光起始位置
 *  输   出:
 *  返回值:
 *  更新时间: 2026年5月26日
 *  备   注:
 ************************************************/
uint8 Flash_choise(uint8 Flash_num[],uint8 Flash_sum,uint8 first_num)
{
        for(int i =first_num;i < first_num+Flash_sum;i++)
        {
            execute_command(Flash_num[i]);
            if(i==Flash_sum-1)
                return 0;
        }
        return 1;
}


/************************************************
 *  函数名称: 语音调度
 *  函数说明:
 *  输入说明: Voice_num为要执行的语音数目0-9个
 *         Voice_sum[]:要执行的语音标志位
 *         first_num为语音起始位置
 *  输   出:
 *  返回值:
 *  更新时间: 2026年5月26日
 *  备   注:
 ************************************************/
uint8 Voice_choise(uint8 Voice_num[],uint8 Voice_sum,uint8 first_num)
{
    for(int i = first_num; i < first_num+Voice_sum; i++)
    {
        if(i > first_num) system_delay_ms(800);  // 连续鸣笛之间留间隔,防止打断前一条
        execute_command(Voice_num[i]);
        if(i == first_num+Voice_sum-1)
            return 0;
    }
    return 1;
}

/************************************************
 *  功能测试: 单动作测试模式
 *  编码器: 选择动作, 编码器按键: 执行, 长按K1: 退出
 ************************************************/
void KM2_ActionTest(void)
{
    /* 动作表: 名称 + 函数指针 */
    typedef struct {
        const char *name;
        uint8 (*func)(void);
    } ActionEntry;

    static const ActionEntry actions[] = {
        {"1.Go 10m       ", Go_straight     },
        {"2.Back 10m     ", Back_straight    },
        {"3.Turn Left    ", Left_straight    },
        {"4.Turn Right   ", Right_straight   },
        {"5.Circle Left  ", Left_circlet     },
        {"6.Circle Right ", Right_circlet    },
        {"7.Go S-Curve   ", Go_S             },
        {"8.Back S-Curve ", Back_S            },
    };
    static const int8 N = sizeof(actions) / sizeof(actions[0]);
    static int8 sel = 0;

    ips200_clear();
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    ips200_show_string(0, 16*17, "Enc:Sel Btn:Run");
    ips200_show_string(0, 16*18, "LongK1:Exit");

    while(1)
    {
        system_delay_ms(10);
        menu_key_scanner();
        if(MENU_KEY_LONG_PRESS == menu_key_get_state(MENU_KEY_1))
        {
            menu_key_clear_state(MENU_KEY_1);
            dispatch_car = 0;
            Back_Motor_SetPwm(0,0);
            Angle_Target_Position = 0;
            break;
        }

        /* 浏览选择 */
        menu_encoder_update();
        if(menu_encoder_has_change())
        {
            int enc = menu_encoder_get_change();
            sel += (int8)((enc > 0) ? 1 : -1);
            if(sel < 0) sel = N - 1;
            if(sel >= N) sel = 0;
        }

        /* 屏幕显示 */
        ips200_show_string(0, 16*1, "Action Test");
        ips200_show_string(0, 16*3, "Sel:");
        ips200_show_int(40, 16*3, sel+1, 1);
        ips200_show_string(55, 16*3, "/");
        ips200_show_int(65, 16*3, N, 1);
        ips200_show_string(0, 16*4, actions[sel].name);

        /* 编码器按键按下时执行动作 */
        static uint8 last_btn = 1;
        uint8 btn_now = gpio_get_level(P11_11);
        if(last_btn == 1 && btn_now == 0)
        {
            ips200_show_string(0, 16*7, "Running...");
            while(actions[sel].func())
            {
                system_delay_ms(10);
                menu_key_scanner();
                if(MENU_KEY_LONG_PRESS == menu_key_get_state(MENU_KEY_1))
                {
                    menu_key_clear_state(MENU_KEY_1);
                    dispatch_car = 0;
                    Back_Motor_SetPwm(0,0);
                    break;
                }
            }
            dispatch_car = 0;
            Back_Motor_SetPwm(0,0);
            Angle_Target_Position = 0;
            ips200_show_string(0, 16*7, "Done!         ");
            Buzzer_check(50);
        }
        last_btn = btn_now;
    }

    dispatch_car = 0;
    ips200_clear();
    menu_set_status("Action test exit");
}

/************************************************
 *  序列编辑器: 可视化 data_store[] 编辑器
 *  编码器: 浏览/指令, K2:插入, K3:删除, K4:保存
 *  长按K1: 退出
 ************************************************/
static uint8 get_phase(uint8 code)
{
    if(code >= 0x04 && code <= 0x0B) return 0;  /* Light */
    if(code >= 0x15 && code <= 0x1E) return 1;  /* Track */
    if(code >= 0x1F && code <= 0x26) return 2;  /* Sport */
    if(code >= 0x0C && code <= 0x14) return 3;  /* Voice */
    return 0;
}

static const char *phase_name(uint8 p)
{
    switch(p) {
        case 0: return "[LIGHT]";
        case 1: return "[TRACK]";
        case 2: return "[SPORT]";
        case 3: return "[VOICE]";
        default: return "[????]";
    }
}

static const char *cmd_name(uint8 code)
{
    switch(code)
    {
        /* Light 0x04-0x0B */
        case 0x04: return "L-Turn On ";
        case 0x05: return "R-Turn On ";
        case 0x06: return "Low Beam  ";
        case 0x07: return "High Beam ";
        case 0x08: return "Fog Light ";
        case 0x09: return "Hazard Flsh";
        case 0x0A: return "Brake Light";
        case 0x0B: return "All Off    ";
        /* Track 0x15-0x1E */
        case 0x15: return "Trk P1-Fwd ";
        case 0x16: return "Trk P1-Fwd2";
        case 0x17: return "Trk P2     ";
        case 0x18: return "Trk P3     ";
        case 0x19: return "Trk P3-R   ";
        case 0x1A: return "Trk P1-Ret ";
        case 0x1B: return "Trk P1-Ret2";
        case 0x1C: return "Trk P2-Ret ";
        case 0x1D: return "Trk P3-Ret ";
        case 0x1E: return "Trk P3-RetR";
        /* Sport 0x1F-0x26 */
        case 0x1F: return "Go 10m     ";
        case 0x20: return "Back 10m   ";
        case 0x21: return "Go S-Curve ";
        case 0x22: return "Back S-Curv";
        case 0x23: return "Circle Left";
        case 0x24: return "Circle Rght";
        case 0x25: return "Turn Left  ";
        case 0x26: return "Turn Right ";
        /* Voice 0x0C-0x14 */
        case 0x0C: return "Horn 1s    ";
        case 0x0D: return "Horn 2s    ";
        case 0x0E: return "Voice 1    ";
        case 0x0F: return "Voice 2    ";
        case 0x10: return "Voice 3    ";
        case 0x11: return "Voice 4    ";
        case 0x12: return "Voice 5    ";
        case 0x13: return "Voice 6    ";
        case 0x14: return "Voice 7    ";
        default:   return "???        ";
    }
}

/* Phase-specific command arrays for constrained editing */
static const uint8 LIGHT_CMDS[] = {0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B};
static const uint8 TRACK_CMDS[] = {0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E};
static const uint8 SPORT_CMDS[] = {0x1F,0x20,0x21,0x22,0x23,0x24,0x25,0x26};
static const uint8 VOICE_CMDS[] = {0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13,0x14};

static const uint8 *PHASE_CMDS[4] = {LIGHT_CMDS, TRACK_CMDS, SPORT_CMDS, VOICE_CMDS};
static const int8 PHASE_SIZES[4] = {8, 10, 8, 9};
/* Recalculate flag counters from data_store */
static void recalc_flags(void)
{
    Flash_sum_flag = 0; Sport_sum_flag = 0; Voice_sum_flag = 0;
    for(int i = 0; i < data_store_count; i++)
    {
        uint8 c = data_store[i];
        if(c >= 0x04 && c <= 0x0B)       Flash_sum_flag++;
        else if(c >= 0x1F && c <= 0x26)  Sport_sum_flag++;
        else if(c >= 0x0C && c <= 0x14)  Voice_sum_flag++;
    }
}

void KM2_EditSequence(void)
{
    static int8 sel = 0;
    static int8 edit_idx = 0;
    static uint8 editing = 0;
    static uint8 cur_phase = 0;

    /* Auto-load from Flash if available */
    if(flash_check(0, 12))
    {
        flash_read_page_to_buffer(0, 12);
        data_store_count = (uint32)flash_union_buffer[0].uint32_type;
        Flash_sum_flag   = (uint8)flash_union_buffer[1].uint32_type;
        Sport_sum_flag   = (uint8)flash_union_buffer[2].uint32_type;
        Voice_sum_flag   = (uint8)flash_union_buffer[3].uint32_type;
        if(data_store_count > 63) data_store_count = 0;
        for(int i = 0; i < data_store_count; i++)
            data_store[i] = (uint8)flash_union_buffer[4 + i].uint32_type;
    }
    if(data_store_count == 0) { data_store_count = 1; data_store[0] = 0x1F; recalc_flags(); }
    cur_phase = get_phase(data_store[sel]);

    ips200_clear();
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    ips200_show_string(0, 16*17, "Enc:Sel Btn:Edit");
    ips200_show_string(0, 16*18, "K2:Ins K3:Del K4:Save LongK1");

    while(1)
    {
        system_delay_ms(10);
        menu_key_scanner();
        if(MENU_KEY_LONG_PRESS == menu_key_get_state(MENU_KEY_1))
        {
            menu_key_clear_state(MENU_KEY_1);
            break;
        }

        /* K2: insert new command (same phase as current) */
        if(key2_flag) { key2_flag = 0;
            if(data_store_count < 63)
            {
                for(int i = data_store_count; i > sel; i--)
                    data_store[i] = data_store[i-1];
                data_store_count++;
                recalc_flags();
                Buzzer_check(30);
            }
        }
        /* K3: delete current command */
        if(key3_flag) { key3_flag = 0;
            if(data_store_count > 1)
            {
                for(int i = sel; i < data_store_count - 1; i++)
                    data_store[i] = data_store[i+1];
                data_store_count--;
                if(sel >= data_store_count) sel = (int8)(data_store_count - 1);
                cur_phase = get_phase(data_store[sel]);
                recalc_flags();
                Buzzer_check(30);
            }
        }
        /* K4: save to Flash */
        if(key4_flag) { key4_flag = 0;
            flash_buffer_clear();
            flash_union_buffer[0].uint32_type = data_store_count;
            flash_union_buffer[1].uint32_type = Flash_sum_flag;
            flash_union_buffer[2].uint32_type = Sport_sum_flag;
            flash_union_buffer[3].uint32_type = Voice_sum_flag;
            for(int i = 0; i < data_store_count; i++)
                flash_union_buffer[4 + i].uint32_type = data_store[i];
            if(flash_check(0, 12))
                flash_erase_page(0, 12);
            flash_write_page_from_buffer(0, 12);
            menu_set_status("Sequence saved to Flash");
            Buzzer_check(100);
        }

        /* Encoder */
        menu_encoder_update();
        if(menu_encoder_has_change())
        {
            int enc = menu_encoder_get_change();
            if(!editing)
            {
                sel += (int8)((enc > 0) ? 1 : -1);
                if(sel < 0) sel = (int8)(data_store_count - 1);
                if(sel >= data_store_count) sel = 0;
                cur_phase = get_phase(data_store[sel]);
            }
            else
            {
                /* Edit mode: only cycle through commands of the SAME phase */
                int8 sz = PHASE_SIZES[cur_phase];
                edit_idx += (int8)((enc > 0) ? 1 : -1);
                if(edit_idx < 0) edit_idx = sz - 1;
                if(edit_idx >= sz) edit_idx = 0;
                data_store[sel] = PHASE_CMDS[cur_phase][edit_idx];
                recalc_flags();
            }
        }

        /* Encoder button: toggle edit mode */
        static uint8 last_btn = 1;
        uint8 btn_now = gpio_get_level(P11_11);
        if(last_btn == 1 && btn_now == 0)
        {
            if(!editing)
            {
                /* Enter: find current cmd index in its phase array */
                cur_phase = get_phase(data_store[sel]);
                for(int i = 0; i < PHASE_SIZES[cur_phase]; i++)
                    if(PHASE_CMDS[cur_phase][i] == data_store[sel])
                        { edit_idx = (int8)i; break; }
                editing = 1;
            }
            else
            {
                editing = 0;
            }
        }
        last_btn = btn_now;

        /* Display */
        ips200_show_string(0, 16*1, "SEQ EDIT");
        ips200_show_string(0, 16*2, "Pos:");
        ips200_show_int(40, 16*2, sel, 2);
        ips200_show_string(60, 16*2, "/");
        ips200_show_int(70, 16*2, data_store_count, 2);
        ips200_show_string(100, 16*2, editing ? "[EDIT]" : "[VIEW]");
        ips200_show_string(0, 16*3, phase_name(cur_phase));
        ips200_show_string(0, 16*4, cmd_name(data_store[sel]));
        ips200_show_int(120, 16*4, data_store[sel], 3);
        ips200_show_string(0, 16*6, "L:");
        ips200_show_int(15, 16*6, Flash_sum_flag, 2);
        ips200_show_string(40, 16*6, "T:");
        ips200_show_int(55, 16*6, (int8)(data_store_count - Flash_sum_flag - Sport_sum_flag - Voice_sum_flag), 2);
        ips200_show_string(80, 16*6, "S:");
        ips200_show_int(95, 16*6, Sport_sum_flag, 2);
        ips200_show_string(120, 16*6, "V:");
        ips200_show_int(135, 16*6, Voice_sum_flag, 2);
    }

    editing = 0;
    ips200_clear();
    menu_set_status("Sequence editor exit");
}
