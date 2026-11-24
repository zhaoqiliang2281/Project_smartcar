/*
 * subject.c
 *
 *  Created on: 2025年6月15日
 *      Author: 吉祥止止
 */

#include "zf_common_headfile.h"

// **************************** 核心循迹 ****************************

int32 TIM_0_flag = 0;

int Point_num=0;            //当前点位

double LAT_data[Point_MAX] = {0};  //纬度存储数组
double LOG_data[Point_MAX] = {0};  //经度存储数组

int home_run[Point_MAX];

                       //点位 1,  2,  3,  4,  5,  6, 7,  8   9    10 11  12 13   14  15
int32 Speed_KM1_Buf[15]  = {300,300,300,300,300,300,300,300,300,300,300,300,300,300,0};//科目1速度限幅//比赛

int32 Speed_KM2_Buf[14]  = {300,300,300,300,300,300,300,300,300,300,300,300,300,0};//科目2速度限幅//比赛


//状态机
int16 next;

/************************************************
 *  函数名称: GPS数组存点：GPS_array(void)
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月16日
 *  备   注：按下按键1存点
 ************************************************/
int i = 0;  //数组索引
void GPS_array(void)
{
    if(key1_flag == 1)
    {
        key1_flag = 0;
        if(Point_MAX <= 19)
        {
            LAT_data[i] = gnss.latitude;         //把实时纬度给存储纬度
            LOG_data[i] = gnss.longitude;        //把实时经度给存储经度

            ips200_show_string(170, 16*9, "Num:");ips200_show_uint(210, 16*9, i+1, 2);
            i++;
            Point_num=i;
        }
        Buzzer_check(50);
    }
}

/************************************************
 *  函数名称: 科目1踩点，存点，读点设置：System_Setting_KM1(void)
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月26日
 *  备   注：按下按键1存点，按下按键2存点，按下按键3读点，
 ************************************************/
double LAT_error=0,LOG_error=0;

void System_Setting_KM1(void)
{
    if(!gpio_get_level(sw1) &&  !gpio_get_level(sw2))//1,2上
    {
        ips200_show_string(230, 16*8, "S");
        GPS_array();
        if(key2_flag == 1)
        {
            key2_flag=0;
            Up_flash_point();
            FLASH_Get_KM1();   //科目一（存flash）
            Buzzer_check(50);
        }
        if(key3_flag == 1)
        {
            key3_flag=0;
            FLASH_READ_KM1();   //科目一(读取flash)
            Read_flash_point();
            Buzzer_check(50);
        }
        if(key4_flag == 1)
        {
            key4_flag=0;
            flash_erase_page(FLASH_SECTION_INDEX, FLASH_GPS_INDEX);   //科目一(擦除页)
            Buzzer_check(50);
            ips200_show_string(172, 16*12, "QI flash");
        }
    }
    else if(gpio_get_level(sw1) &&  gpio_get_level(sw2))//1,2下
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

            };
            user_gps_display_init(IPS200_TYPE,10,10,220,300);
            user_gps_transition(gps_point_test,sizeof(gps_point_test) / sizeof(gps_point_test[0]));
            user_gps_display(RGB565_PURPLE);
            Buzzer_check(50);
        }
//        if(key2_flag == 1)
//        {
//            key2_flag=0;
//
//            Buzzer_check(50);
//        }
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
    if(!gpio_get_level(sw1) && gpio_get_level(sw2))  //1上.2下
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
 *  函数名称: 科目2踩点，存点，读点设置：System_Setting_KM2(void)
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月26日
 *  备   注：按下按键1存点，按下按键2存点，按下按键3读点，
 ************************************************/
void System_Setting_KM2(void)
{
    if(!gpio_get_level(sw1) &&  !gpio_get_level(sw2))//1,2上
    {
        ips200_show_string(230, 16*8, "S");
        GPS_array();
        if(key2_flag == 1)
        {
            key2_flag=0;
            Up_flash_point();
            FLASH_Get_KM2();   //科目二（存flash）
            Buzzer_check(50);
        }
        if(key3_flag == 1)
        {
            key3_flag=0;
            FLASH_READ_KM2();   //科目二(读取flash)
//            GPS_Remote();//gps除零漂
            Read_flash_point();
            Buzzer_check(50);
        }
        if(key4_flag == 1)
        {
            key4_flag=0;
            flash_erase_page(FLASH_SECTION_INDEX1, FLASH_GPS_INDEX1);   //科目二(擦除页)
            Buzzer_check(50);
            ips200_show_string(172, 16*12, "QI flash");
        }
    }
    else if(gpio_get_level(sw1) &&  gpio_get_level(sw2))//1,2下
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
                {LAT_data_go[12],LOG_data_go[12]},//科二13个点
                {LAT_data_go[13],LOG_data_go[13]},//科二14个点
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
 *  函数名称: 科目3：System_Setting_KM3(void)
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年3月28日
 *  备   注：
 ************************************************/
void System_Setting_KM3(void)
{

}

/************************************************
 *  函数名称: 科目1循迹：Follow_track1(void)
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月26日
 *  备   注：
 ************************************************/

int8 Back_Motor_PID_Start_flag = 0;//电机PID控制选择标志位
int8 Back_Motor_spin_PID_Start_flag = 0;//旋转PID控制选择标志位

extern uint8 dispatch_car; //发车标志位(main定义，发车按键按下，1秒后发车，科目完成后标志位清0)

int32 Back_Motor_Set_Speed = 0;    //后驱电机速度设置大小

uint8 Dri_Control_flag = 0;

void Follow_track1(void)
{
    get_point_distance();       //把实时位置赋给home_run[L]，用于对比实时对比
    GPS_Two_point_error();      //计算方向误差
    GPS_Two_point_Distance();   //要计算两点距离，用于实现距离PID

    ips200_show_int(0, 100, next, 2);//显示实时到达点位

    if(home_run[next] < need_distance  && home_run[1]!=0 && next==1)
    {
        next=2;
        Back_Motor_PID_Start_flag = 1; //为1时开启后驱电机速度环PID控制
        Back_Motor_spin_PID_Start_flag = 0;

        Dri_Control_flag = 0;   //开启正方向GPS与陀螺仪控制前转向电机

        Back_Motor_Set_Speed = Speed_KM1_Buf[0];//设置速度 //由数组统一赋值各个点的速度
        Buzzer_SetTimer(50);
    }
    if(home_run[next] < need_distance  && next == 2)
    {
        next = 3;
        Back_Motor_PID_Start_flag = 1;
        Back_Motor_Set_Speed = Speed_KM1_Buf[1];//设置速度
        Buzzer_SetTimer(50);
    }
    if(home_run[next] <need_distance  && next == 3)
    {
        next = 4;
        Back_Motor_Set_Speed = Speed_KM1_Buf[2];//设置速度
        Buzzer_SetTimer(50);
    }
    if(home_run[next] <need_distance  && next ==4)
    {
        next = 5;
        Back_Motor_Set_Speed = Speed_KM1_Buf[3];//设置速度
        Buzzer_SetTimer(50);
    }
    if(home_run[next] <need_distance  && next == 5)
    {
        next = 6;
        Back_Motor_Set_Speed = Speed_KM1_Buf[4];//设置速度
        Buzzer_SetTimer(50);
    }
    if(home_run[next] <need_distance  && next == 6)
    {
        next = 7;
        Back_Motor_Set_Speed = Speed_KM1_Buf[5];//设置速度
        Buzzer_SetTimer(50);
    }
    if(home_run[next] <need_distance  && next == 7)
    {
        next = 8;
        Back_Motor_Set_Speed = Speed_KM1_Buf[6];//设置速度
        Buzzer_SetTimer(50);
    }

    if(home_run[next] <need_distance  && next == 8)
    {
        next = 9;
        Back_Motor_Set_Speed = Speed_KM1_Buf[7];//设置速度
        Buzzer_SetTimer(50);
    }
    if(home_run[next] <need_distance  && next == 9)
    {
        next = 10;
        Back_Motor_Set_Speed = Speed_KM1_Buf[8];//设置速度
        Buzzer_SetTimer(50);
    }
    if(home_run[next] <need_distance  && next == 10)
    {
        next = 11;
        Back_Motor_Set_Speed = Speed_KM1_Buf[9];//设置速度
        Buzzer_SetTimer(50);
    }
    if(home_run[next] <need_distance  && next == 11)
    {
        next = 12;
        Back_Motor_Set_Speed = Speed_KM1_Buf[10];//设置速度
        Buzzer_SetTimer(50);
    }
    if(home_run[next] <need_distance  && next == 12)
    {
        next = 13;
        Back_Motor_Set_Speed = Speed_KM1_Buf[12];//设置速度
        Buzzer_SetTimer(50);
    }
    if(home_run[next] <need_distance  && next == 13)
    {
        Buzzer_SetTimer(50);

        next = 14;
        Back_Motor_Set_Speed = Speed_KM1_Buf[12];//设置速度
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
//            Dri_Control_flag = 1;   //开启 反 方向GPS与陀螺仪控制前转向电机
//
//            next = 14;
//            Back_Motor_Set_Speed = -300;//设置速度Speed_KM1_Buf[12]
//            Buzzer_SetTimer(50);
//        }
    }

    if(home_run[next] <need_distance  && next == 14)
    {
        next = 15;
        delay_ScanTemp = 0;//关闭定时器延迟发车标志位//科目完成清除发车延迟的标志位
        dispatch_car = 0;//关闭发车标志位
        Back_Motor_Set_Speed = Speed_KM1_Buf[14];//设置速度
        Back_Motor_PID_Start_flag = 0; //关闭位置PID，给电机速度为0
        Motor_Start = 0;
        Buzzer_SetTimer(1000);
    }
    //电机速度模式选择
    if(Back_Motor_PID_Start_flag == 1 && dispatch_car ==1  && Motor_Start == 1) //flag_speed如果是0，开启PID
    {
        if(Target_Position>750)//增加转向差速
        {
            Back_Motor_SetPwm(Back_Motor_Set_Speed-100,Back_Motor_Set_Speed+100);      //电机速度赋值
        }
        else if(Target_Position<-750)
        {
            Back_Motor_SetPwm(Back_Motor_Set_Speed+100,Back_Motor_Set_Speed-100);      //电机速度赋值
        }
        else
        {
            Back_Motor_SetPwm(Back_Motor_Set_Speed,Back_Motor_Set_Speed);      //电机速度赋值
        }
    }
    else if(Back_Motor_spin_PID_Start_flag == 1 && dispatch_car ==1 && Motor_Start == 1)
    {                       //you
        Back_Motor_SetPwm(-Diversion_PID.Result-100,Diversion_PID.Result+100);   //
    }
    else if(Motor_Start == 0)
    {
        Back_Motor_SetPwm(0,0);   //否则速度为0
    }
    //发车后开启
    if(dispatch_car == 1 && Dri_Control_flag == 0)  //开启 正 方向GPS与陀螺仪控制前转向电机
    {
        Control_straight(); //GPS与陀螺仪误差计算PID控制
    }
    else if(dispatch_car == 1 && Dri_Control_flag == 1) //开启 反 方向GPS与陀螺仪控制前转向电机
    {
        Dir_Control_straight();
    }
    else
    {
        //不给GPS与陀螺仪控制
    }
//    printf("%d,%d,%d\n\r",(int)Back_Motor_PID_Start_flag,(int)dispatch_car,(int)Motor_Start);//上位机看值

}

/************************************************
 *  函数名称: 科目2循迹：Follow_track2(void)
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月26日
 *  备   注：
 ************************************************/
void Follow_track2(void)
{
    //科目二
    get_point_distance();       //把实时位置赋给home_run[L]，用于对比实时对比
    GPS_Two_point_error();      //GPS解析，计算方向误差
    GPS_Two_point_Distance();   //要计算两点距离，用于实现距离PID

    ips200_show_int(0, 100, next, 2);//显示实时到达点位

}

/************************************************
 *  函数名称: 科目3循迹：Follow_track3(void)
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月26日
 *  备   注：
 ************************************************/
void Follow_track3(void)//科目三
{
    //左右
    YDir_Speed_PID_Start(1,x,120);
    Target_Position = -YDir_Speed_PID.Result;
    ips200_show_int( 0, 16*5, Target_Position, 5);
    ips200_show_int( 0, 16*6, YDir_Speed_PID.Result, 5);

    //距离
    DDir_Speed_PID_Start(1,z,35);
    ips200_show_int( 0, 16*7, z, 5);
    ips200_show_int( 0, 16*8, DDir_Speed_PID.Result, 5);

    Back_Motor_SetPwm(-DDir_Speed_PID.Result,-DDir_Speed_PID.Result);

    printf("%d,%d,%d\n\r",(int)y,YDir_Speed_PID.Result,(int)Target_Position);/*上位机看值,调参代码，勿删*/

}

