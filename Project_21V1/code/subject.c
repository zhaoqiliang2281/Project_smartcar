/*
 * subject.c
 *
 *  Created on: 2025年6月15日
 *      Author: LiangZhaoqi
 */

#include "zf_common_headfile.h"

int32 TIM_0_flag = 0;

double LAT_data[Point_MAX] = {0};  //纬度存储数组
double LOG_data[Point_MAX] = {0};  //经度存储数组

float KM2_INS_X[16] = {0};
float KM2_INS_Y[16] = {0};

double home_run[Point_MAX];

                       //点位 1,  2,  3,  4,  5,  6, 7,  8   9    10 11  12 13   14  15
int32 Speed_KM1_Buf[15]  = {300,300,300,300,300,300,300,300,300,300,300,300,300,300,0};//目标1速度限制//调整

int32 Speed_KM2_Buf[16]  = {300,300,300,300,300,300,300,300,300,300,300,300,200,200,200,0};//目标2速度限制//调整

//状态变量
int16 next;

/************************************************
 *  函数名称: GPS数据存储
 *  函数说明:
 *  输入说明:
 *  输   出:
 *  返回值:
 *  更新时间: 2025年6月16日
 *  备   注: 包含路径点1路径点
 ************************************************/
static int16 i = 0;  //存储计数器
void GPS_array(void)
{
    if(key1_flag == 1)
    {
        key1_flag = 0;
        if(i < Point_MAX)
        {
            LAT_data[i] = gnss.latitude;
            LOG_data[i] = gnss.longitude;
            if(i < 16) { KM2_INS_X[i] = INS_X; KM2_INS_Y[i] = INS_Y; }
            ips200_show_string(0, 16*6, "Rec:"); ips200_show_int(30, 16*6, i+1, 2);
            ips200_show_string(55, 16*6, "/16");
            ips200_show_string(95, 16*6, km2_pt_name(i));
            i++;
        }
        Buzzer_check(50);
    }
}

void reset_gps_index(void) { i = 0; }

void gps_index_undo(void) { if(i > 0) i--; }
int16 get_gps_index(void) { return i; }

const char *km2_pt_name(int16 idx)
{
    switch(idx)
    {
        case 0:  return "Start       ";
        case 1:  return "D1L-In      ";
        case 2:  return "D1L-Out     ";
        case 3:  return "D1-In       ";
        case 4:  return "D1-Out      ";
        case 5:  return "D2-In       ";
        case 6:  return "D2-Out      ";
        case 7:  return "D3-In       ";
        case 8:  return "D3-Out      ";
        case 9:  return "D3R-In      ";
        case 10: return "D3R-Out     ";
        case 11: return "Transit     ";
        case 12: return "Action Area ";
        case 13: return "Return-Pt1  ";
        case 14: return "Return-Pt2  ";
        case 15: return "Finish      ";
        default: return "---         ";
    }
}

/************************************************
 *  函数名称: 目标1路径点，包含路径点设置
 *  函数说明:
 *  输入说明:
 *  输   出:
 *  返回值:
 *  更新时间: 2025年6月26日
 *  备   注: 包含路径点1路径点，包含路径点2路径点，包含第3段，
 ************************************************/
double LAT_error=0,LOG_error=0;

void System_Setting_KM1(void)
{
    if(!gpio_get_level(sw1) &&  !gpio_get_level(sw2))//1,2关
    {
        ips200_show_string(230, 16*8, "S");
        GPS_array();
        if(key2_flag == 1)
        {
            key2_flag=0;
            Up_flash_point();
            FLASH_Get_KM1();   //将目标一存入flash中
            Buzzer_check(50);
        }
        if(key3_flag == 1)
        {
            key3_flag=0;
            FLASH_READ_KM1();   //读取目标一(读取flash)
            Read_flash_point();
            Buzzer_check(50);
        }
        if(key4_flag == 1)
        {
            key4_flag=0;
//            flash_erase_page(FLASH_SECTION_INDEX, FLASH_GPS_INDEX);   //擦除目标一(擦除页)
            Buzzer_check(50);
//            ips200_show_string(172, 16*12, "QI flash");
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
                {LAT_data_go[13],LOG_data_go[13]},
                {LAT_data_go[14],LOG_data_go[14]},
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
            delay_Start = 1;    //开启定时器1s延时
//            Km2_protect_flag = 1;
//                execute_command(8);
        }
        if(Timer_delay_ms(1000)==1) //延时函数到达1s延时后执行
        {
            dispatch_car = 1;           //开启转向标志位
            delay_ScanTemp = 0;         //关闭延时1标志位
        }
        if(dispatch_car == 1 && next == 1 && Transform == 1)  //开始启动速度
        {
               Back_Motor_SetPwm(300,300);
        }
//        if(key3_flag == 1)
//        {
//            key3_flag=0;
//
//            Buzzer_check(50);
//        }
//        if(key4_flag == 1)
//        {
//            key4_flag=0;
//
//            Buzzer_check(50);
//        }
    }
    if(!gpio_get_level(sw1) && gpio_get_level(sw2))  //1关.2开
    {
        if(key3_flag == 1)
        {
            key3_flag=0;
            Buzzer_SetTimer(50);
            LAT_error=gnss.latitude-LAT_data_go[0];
            LOG_error=gnss.longitude-LOG_data_go[0];
            for(int i=0;i<15;i++)
            {
                LAT_data_go[i]+=LAT_error;
                LOG_data_go[i]+=LOG_error;
            }
        }
    }
}

/************************************************
 *  函数名称: 目标2路径点，包含路径点设置
 *  函数说明:
 *  输入说明:
 *  输   出:
 *  返回值:
 *  更新时间: 2025年6月26日
 *  备   注: 包含路径点1路径点，包含路径点2路径点，包含第3段，
 ************************************************/
void System_Setting_KM2(void)
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
        if(key4_flag == 1)
        {
            key4_flag=0;
            flash_erase_page(FLASH_SECTION_INDEX1, FLASH_GPS_INDEX1);   //擦除目标(擦除页)
            Buzzer_check(50);
            ips200_show_string(172, 16*12, "QI flash");
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
                {LAT_data_go[12],LOG_data_go[12]},//索引13个点
                {LAT_data_go[13],LOG_data_go[13]},//索引14个点
                {LAT_data_go[14],LOG_data_go[14]},
            };
            user_gps_display_init(IPS200_TYPE,10,10,220,300);
            user_gps_transition(gps_point_test,sizeof(gps_point_test) / sizeof(gps_point_test[0]));
            user_gps_display(RGB565_PURPLE);
            Buzzer_check(50);
        }
    }
}

/************************************************
 *  函数名称: 目标3
 *  函数说明:
 *  输入说明:
 *  输   出:
 *  返回值:
 *  更新时间: 2026年3月28日
 *  备   注:
 ************************************************/
void System_Setting_KM3(void)
{

}

/************************************************
 *  函数名称: 目标1循迹
 *  函数说明:
 *  输入说明:
 *  输   出:
 *  返回值:
 *  更新时间: 2025年6月26日
 *  备   注:
 ************************************************/

int8 Back_Motor_PID_Start_flag = 0;     //开启PID方向选择标志位
int8 Back_Motor_spin_PID_Start_flag = 0;//旋转PID方向选择标志位

extern uint8 dispatch_car;          //转向标志位(main中定义，用于指示是否启动转向控制，1为启动，标志位为0)

int32 Back_Motor_Set_Speed = 0;     //后轮速度设定值

uint8 Dri_Control_flag = 0;

void Follow_track1(void)
{
    get_point_distance();       //将实时位置存入home_run[L]中对比实时距离
    GPS_Two_point_error();      //计算方位角度
    GPS_Two_point_Distance();   //要使用距离作为条件，使用距离标志开启PID

    ips200_show_int(0, 100, next, 2);//显示实时点序号

    if(home_run[next] < need_distance  && home_run[1]!=0 && next==1)
    {
        next=2;
        Back_Motor_PID_Start_flag = 1; //为1时开启位置控制速度PID控制
        Back_Motor_spin_PID_Start_flag = 0;

        Dri_Control_flag = 0;   //关闭 不使用GPS方位控制前轮转向

        Back_Motor_Set_Speed = Speed_KM1_Buf[0];//前进速度 //参数统一赋值改变速度
        Buzzer_SetTimer(50);
    }
    if(home_run[next] < need_distance  && next == 2)
    {
        next = 3;
        Back_Motor_PID_Start_flag = 1;
        Back_Motor_Set_Speed = Speed_KM1_Buf[1];//前进速度
        Buzzer_SetTimer(50);
    }
    if(home_run[next] <need_distance  && next == 3)
    {
        next = 4;
        Back_Motor_Set_Speed = Speed_KM1_Buf[2];//前进速度
        Buzzer_SetTimer(50);
    }
    if(home_run[next] <need_distance  && next ==4)
    {
        next = 5;
        Back_Motor_Set_Speed = Speed_KM1_Buf[3];//前进速度
        Buzzer_SetTimer(50);
    }
    if(home_run[next] <need_distance  && next == 5)
    {
        next = 6;
        Back_Motor_Set_Speed = Speed_KM1_Buf[4];//前进速度
        Buzzer_SetTimer(50);
    }
    if(home_run[next] <need_distance  && next == 6)
    {
        next = 7;
        Back_Motor_Set_Speed = Speed_KM1_Buf[5];//前进速度
        Buzzer_SetTimer(50);
    }
    if(home_run[next] <need_distance  && next == 7)
    {
        next = 8;
        Back_Motor_Set_Speed = Speed_KM1_Buf[6];//前进速度
        Buzzer_SetTimer(50);
    }

    if(home_run[next] <need_distance  && next == 8)
    {
        next = 9;
        Back_Motor_Set_Speed = Speed_KM1_Buf[7];//前进速度
        Buzzer_SetTimer(50);
    }
    if(home_run[next] <need_distance  && next == 9)
    {
        next = 10;
        Back_Motor_Set_Speed = Speed_KM1_Buf[8];//前进速度
        Buzzer_SetTimer(50);
    }
    if(home_run[next] <need_distance  && next == 10)
    {
        next = 11;
        Back_Motor_Set_Speed = Speed_KM1_Buf[9];//前进速度
        Buzzer_SetTimer(50);
    }
    if(home_run[next] <need_distance  && next == 11)
    {
        next = 12;
        Back_Motor_Set_Speed = Speed_KM1_Buf[10];//前进速度
        Buzzer_SetTimer(50);
    }
    if(home_run[next] <need_distance  && next == 12)
    {
        next = 13;
        Back_Motor_Set_Speed = Speed_KM1_Buf[12];//前进速度
        Buzzer_SetTimer(50);
    }
    if(home_run[next] <need_distance  && next == 13)
    {
        Buzzer_SetTimer(50);

        next = 14;
        Back_Motor_Set_Speed = Speed_KM1_Buf[12];//前进速度
        Buzzer_SetTimer(50);

//        Back_Motor_PID_Start_flag = 0;
//        Back_Motor_spin_PID_Start_flag = 1;
//
//        Diversion_PID_Start(1,Yaw,270);
//
//        if(Yaw <272 && Yaw > 268)
//        {
//            Back_Motor_PID_Start_flag =1;
//
//            Back_Motor_spin_PID_Start_flag = 0;
//
//            Diversion_PID_Start(0,0,0);
//
//            Dri_Control_flag = 1;   //开启 使用GPS方位控制前轮转向
//
//            next = 14;
//            Back_Motor_Set_Speed = -300;//前进速度Speed_KM1_Buf[12]
//            Buzzer_SetTimer(50);
//        }
    }

    if(home_run[next] <need_distance  && next == 14)
    {
        next = 15;
        delay_ScanTemp = 0;//关闭定时器标志位//目标到达后停止寻迹标志位
        dispatch_car = 0;//关闭转向标志位
        Back_Motor_Set_Speed = Speed_KM1_Buf[14];//前进速度
        Back_Motor_PID_Start_flag = 0; //关闭位置PID后速度归零
        Angle_Target_Position = 0;  // 方向盘回正
        Motor_Start = 0;
        Buzzer_SetTimer(1000);
    }
    //转向控制
    if(dispatch_car == 1 && Dri_Control_flag == 0)  //关闭 使用GPS方位控制前轮转向
    {
        Control_straight((Back_Motor_Set_Speed >= 0) ? 1 : -1); //GPS方位控制前轮PID控制
    }
    else if(dispatch_car == 1 && Dri_Control_flag == 1) //开启 使用GPS方位控制前轮转向
    {
        Control_straight((Back_Motor_Set_Speed >= 0) ? 1 : -1);
    }
    else
    {
        //不使用GPS方位控制
    }

    if(Back_Motor_PID_Start_flag == 1 && dispatch_car ==1  && Motor_Start == 1)
    {
        Back_Motor_SetPwm_Diff(Back_Motor_Set_Speed, Error);
    }
    else if(Back_Motor_spin_PID_Start_flag == 1 && dispatch_car ==1 && Motor_Start == 1)
    {
    }
    else if(Motor_Start == 0)
    {
        Back_Motor_SetPwm(0,0);
    }

}

/************************************************
 *  函数名称: 目标2循迹
 *  函数说明:
 *  输入说明:
 *  输   出:
 *  返回值:
 *  更新时间: 2025年6月26日
 *  备   注:
 ************************************************/
void Follow_track2(void)
{
    //目标点
    get_point_distance();       //将实时位置存入home_run[L]中对比实时距离
    GPS_Two_point_error();      //GPS计算方位角度
    GPS_Two_point_Distance();   //要使用距离作为条件，使用距离标志开启PID
    ips200_show_int(0, 100, next, 2);//显示实时点序号

}
