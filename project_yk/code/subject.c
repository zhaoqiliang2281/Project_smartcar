/*
 * subject.c
 *
 *  Created on: 2025年6月15日
 *      Author: 吉祥止止
 */

#include "zf_common_headfile.h"

// **************************** 核心循迹 ****************************
/**
  * @brief  gps点位的循迹
  * @param  home                点位
  * @param  need_distance       距离点位的距离
  * @param  next                到达的点位
  * @retval 无
  */

int32 TIM_0_flag = 0;

int Point_num=0;            //当前点位

double LAT_data[Point_MAX] = {0};  //纬度存储数组
double LOG_data[Point_MAX] = {0};  //经度存储数组

int home_run[Point_MAX];

                        // 1, 2, 3, 4, 5, 6, 7, 8
uint8 Speed_KM1_Buf[8]  = {30,15,15,15,40,0,0,0};//科目1速度限幅//比赛

                        // 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,11,12,13,14
uint8 Speed_km2_Buf[14] = {20,20,20,20,50,20,20,30,10,20,20,20,50,0};//科目2速度限幅

uint8 Speed_km2_1_Buf[14] = {20,20,20,20,50,20,20,25,10,20,20,20,50,0};//科目2速度限幅

uint8 Speed_km2_2_Buf[14] = {15,15,15,15,50,20,20,30,10,15,15,15,50,0};;//科目2速度限幅

////默认坡道                // 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,11,12,13,14
//uint8 Speed_km3_Buf[14] = {30,20,30,25,15,20,15,25,10,13,30,0,0,0};//科目3速度限幅

//默认坡道                // 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,11,12,13,14
uint8 Speed_km3_Buf[14] = {20,20,20,20,15,15,20,30,0,0,0,0,0,0};//科目3速度限幅

//桥洞                    // 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,11,12,13,14
uint8 Speed_km3_Buf1[14] = {10,13,25,30,15,20,15,25,15,30,30,0,0,0};//科目3速度限幅
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
    //在页面上展示时时经纬度//航向角
    ips200_show_string(0, 16*13, "LAT:");  ips200_show_float(60, 16*13, gnss.latitude, 4, 6);// 纬度
    ips200_show_string(0, 16*14, "LOG:");  ips200_show_float(60, 16*14, gnss.longitude, 4, 6);// 经度

    if(key1_flag == 1)
    {
        key1_flag = 0;
        if(Point_MAX <= 19)
        {
            LAT_data[i] = gnss.latitude;         //把实时纬度给存储纬度
            LOG_data[i] = gnss.longitude;        //把实时经度给存储经度

            ips200_show_string(170, 16*9, "Num:");     ips200_show_uint(210, 16*9, i+1, 2);

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
            next = 0;
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
            };
            user_gps_display_init(IPS200_TYPE,10,10,220,300);
            user_gps_transition(gps_point_test,sizeof(gps_point_test) / sizeof(gps_point_test[0]));
            user_gps_display(RGB565_PURPLE);
            Buzzer_check(50);
        }
        if(key2_flag == 1)
        {
            key2_flag=0;

            Buzzer_check(50);
        }
        if(key3_flag == 1)
        {
            key3_flag=0;

            Buzzer_check(50);
        }
        if(key4_flag == 1)
        {
            key4_flag=0;

            Buzzer_check(50);
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
            next = 0;
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
            };
            user_gps_display_init(IPS200_TYPE,10,10,220,300);
            user_gps_transition(gps_point_test,sizeof(gps_point_test) / sizeof(gps_point_test[0]));
            user_gps_display(RGB565_PURPLE);
            Buzzer_check(50);
        }
        if(key2_flag == 1)
        {
            key2_flag=0;

            Buzzer_check(50);
        }
        if(key3_flag == 1)
        {
            key3_flag=0;

            Buzzer_check(50);
        }
        if(key4_flag == 1)
        {
            key4_flag=0;

            Buzzer_check(50);
        }
    }
}

/************************************************
 *  函数名称: 科目3踩点，存点，读点设置：System_Setting_KM3(void)
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月26日
 *  备   注：按下按键1存点，按下按键2存点，按下按键3读点，
 ************************************************/
void System_Setting_KM3(void)
{
    ips200_show_string(230, 16*8, "S");
    if(!gpio_get_level(sw1) &&  !gpio_get_level(sw2))//1,2上
    {
        GPS_array();
        if(key2_flag == 1)
        {
            key2_flag=0;
            Up_flash_point();
            FLASH_Get_KM3();   //科目三（存flash）
            Buzzer_check(50);
        }
        if(key3_flag == 1)
        {
            key3_flag=0;
            FLASH_READ_KM3();   //科目三(读取flash)
            Read_flash_point();
            Buzzer_check(50);
        }
        if(key4_flag == 1)
        {
            key4_flag=0;
            flash_erase_page(FLASH_SECTION_INDEX2, FLASH_GPS_INDEX2);   //科目三(擦除页)
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
            next = 0;
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
        if(key2_flag == 1)
        {
            key2_flag=0;

            Buzzer_check(50);
        }
        if(key3_flag == 1)
        {
            key3_flag=0;

            Buzzer_check(50);
        }
        if(key4_flag == 1)
        {
            key4_flag=0;

            Buzzer_check(50);
        }
    }
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
int8 Speed_duty;    //舵机PID标志位
int8 flag_speed = 0;//电机PID控制选择标志位
extern uint8 dispatch_car; //发车标志位(主循环定义，发车按键按下，1秒后发车，科目完成后标志位清0)

uint8 KM3weizixuanze = 0;
void Follow_track1(void)
{
    /*
    //科目一
    get_point_distance();       //把实时位置赋给home_run[L]，用于对比实时对比
    GPS_Two_point_error();      //计算方向误差
    GPS_Two_point_Distance();   //要计算两点距离，用于实现距离PID

    ips200_show_int(0, 100, next, 2);//显示实时到达点位

    Pwm_Motor(Speed_duty);      //电机速度赋值

    if(home_run[next] < need_distance  && home_run[0]!=0 && next==0)
    {
        next=1;
        Speed_duty=30;
    }
    if(home_run[next] < need_distance  && next == 1)
    {
        next = 2;
        Speed_duty=35;
    }
    if(home_run[next] <need_distance  && next == 2)
    {
        next = 3;
        Speed_duty=10;
    }
    if(home_run[next] <need_distance  && next ==3)
    {
        next = 4;
        Speed_duty=10;
    }
    if(home_run[next] <need_distance  && next == 4)
    {
        next = 5;
        Speed_duty=10;
    }
    if(home_run[next] <need_distance  && next == 5)
    {
        next = 6;
        Speed_duty=10;
    }
    if(home_run[next] <need_distance  && next == 6)
    {
        next = 7;
        Speed_duty=40;
    }
    if(home_run[next] <need_distance  && next == 7)
    {
        next = 8;
        Speed_duty=0;
    }
    //舵机PID选择是否开启
    if(Speed_duty != 0) //如果速度不为0，进行舵机PID
    {
        Control_straight(); //舵机PID控制
    }
    else    //否则舵机默认归中
    {
        Steer_Set(SERVO_MOTOR_MID);//给舵机95度
    }
    */
    get_point_distance();       //把实时位置赋给home_run[L]，用于对比实时对比
    GPS_Two_point_error();      //计算方向误差
    GPS_Two_point_Distance();   //要计算两点距离，用于实现距离PID

    ips200_show_int(0, 100, next, 2);//显示实时到达点位

    if(home_run[next] < need_distance  && home_run[0]!=0 && next==0)
    {
        next=1;
        Speed_duty=20;  //速度不为0，开启舵机PID
        flag_speed = 0; //为0时开启位置PID
        Speed_PID_Init(Speed_KM1_Buf[0]);//速度PID限幅
        Buzzer_SetTimer(50);
    }
    if(home_run[next] < need_distance  && next == 1)
    {
        next = 2;
        Speed_PID_Init(Speed_KM1_Buf[1]);
        Buzzer_SetTimer(50);
    }
    if(home_run[next] <need_distance  && next == 2)
    {
        next = 3;
        Speed_PID_Init(Speed_KM1_Buf[2]);
        Buzzer_SetTimer(50);
    }
    if(home_run[next] <need_distance  && next ==3)
    {
        next = 4;
        Speed_PID_Init(Speed_KM1_Buf[3]);
        Buzzer_SetTimer(50);
    }
    if(home_run[next] <need_distance  && next == 4)
    {
        next = 5;
        Speed_PID_Init(Speed_KM1_Buf[4]);
        Buzzer_SetTimer(50);
    }
    if(home_run[next] <need_distance  && next == 5)
    {
        next = 6;
        Speed_PID_Init(Speed_KM1_Buf[5]);
        Buzzer_SetTimer(50);
    }
    if(home_run[next] <need_distance  && next == 6)
    {
        next = 7;
        Speed_PID_Init(Speed_KM1_Buf[6]);
        Buzzer_SetTimer(50);
    }
    if(home_run[next] <need_distance  && next == 7)
    {
        next = 8;
        delay_ScanTemp = 0;//关闭定时器延迟发车标志位
        dispatch_car = 0;//关闭发车标志位
        Speed_duty=0;   //关闭舵机PID
        flag_speed = 1; //关闭位置PID，给电机速度为0
        Buzzer_SetTimer(1000);
    }
    //电机速度模式选择
    if(flag_speed == 0 && dispatch_car ==1) //flag_speed如果是0，开启PID
    {
        Pwm_Motor(resultSpeed);      //电机速度赋值
    }
    else
    {
        Pwm_Motor(0);   //否则速度为0
    }

    //舵机PID选择是否开启
    if(Speed_duty != 0) //如果速度不为0，进行舵机PID
    {
        Control_straight(); //舵机PID控制
    }
    else    //否则舵机默认归中
    {
        Steer_Set(SERVO_MOTOR_MID);//给舵机95度
    }

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

    if(home_run[next] < need_distance  && home_run[next]!=0 && next==0)
    {
        next=1;
        Speed_duty=20;  //速度不为0，开启舵机PID
        flag_speed = 0; //为0时开启位置PID
        Speed_PID_Init(Speed_km2_Buf[0]); //速度PID限幅
        Buzzer_SetTimer(50);
    }
    if(home_run[next] < need_distance  && next == 1)
    {
        next = 2;
        flag_speed = 0;
        Speed_PID_Init(Speed_km2_Buf[1]);
        Buzzer_SetTimer(50);
    }
    if(home_run[next] <need_distance  && next == 2)
    {
        next = 3;
        flag_speed = 0;
        Speed_PID_Init(Speed_km2_Buf[2]);
        Buzzer_SetTimer(50);
    }
    if(home_run[next] <need_distance  && next == 3)
    {
        next = 4;
        flag_speed = 0;
        Speed_PID_Init(Speed_km2_Buf[3]);
        Buzzer_SetTimer(50);
    }
    if(home_run[next] <need_distance  && next == 4)
    {
        next = 5;
        flag_speed = 0;
        Speed_PID_Init(Speed_km2_Buf[4]);
        Buzzer_SetTimer(50);
    }
    if(home_run[next] < need_distance  && next==5)
    {
        next = 6;
        flag_speed = 0;
        Speed_PID_Init(Speed_km2_Buf[5]);
        Buzzer_SetTimer(50);
    }

    if(home_run[next] < need_distance  && next == 6)
    {
        next = 7;
        flag_speed = 0;
        Speed_PID_Init(Speed_km2_Buf[6]);
        Buzzer_SetTimer(50);
    }
    if(home_run[next] <need_distance  && next == 7)
    {
        next = 8;
        flag_speed = 0;
        Speed_PID_Init(Speed_km2_Buf[7]);
        Buzzer_SetTimer(50);
    }
    if(home_run[next] <need_distance  && next == 8)//
    {
        next = 9;
        flag_speed = 0;
        Speed_PID_Init(Speed_km2_Buf[8]);
        Buzzer_SetTimer(50);
    }
    if(home_run[next] <need_distance  && next == 9)
    {
        next = 10;
        flag_speed = 0;
        Speed_PID_Init(Speed_km2_Buf[9]);
        Buzzer_SetTimer(50);
    }
    if(home_run[next] <need_distance  && next == 10)
    {
        next = 11;
        flag_speed = 0;
        Speed_PID_Init(Speed_km2_Buf[10]);
        Buzzer_SetTimer(50);
    }
    if(home_run[next] <need_distance  && next == 11)
    {
        next = 12;
        flag_speed = 0;
        Speed_PID_Init(Speed_km2_Buf[11]);
        Buzzer_SetTimer(50);
    }
    if(home_run[next] <need_distance  && next == 12)
    {
        next = 13;
        flag_speed = 0;
        Speed_PID_Init(Speed_km2_Buf[12]);
        Buzzer_SetTimer(50);
    }
    if(home_run[next] <need_distance  && next == 13)
    {
        next = 14;
        delay_ScanTemp = 0;
        dispatch_car = 0;//关闭发车标志位
        Speed_duty=0;    //关闭舵机PID
        flag_speed = 1;  //关闭位置PID，给电机速度为0
        Buzzer_SetTimer(1000);
    }

    //电机速度模式选择
    if(flag_speed == 0 && dispatch_car ==1) //flag_speed如果是0，开启PID
    {
        Pwm_Motor(resultSpeed);      //电机速度赋值
    }
    else
    {
        Pwm_Motor(0);   //否则速度为0
    }

//    //电机速度模式选择
//    if(flag_speed == 0 && dispatch_car ==1) //flag_speed如果是0，开启PID
//    {
//        Pwm_Motor(resultSpeed);      //电机速度赋值
//    }
//    if(flag_speed == 1)
//    {
//        Pwm_Motor(0);   //否则速度为0
//    }


    //舵机PID选择是否开启
    if(Speed_duty != 0) //如果速度不为0，进行舵机PID
    {
        Control_straight(); //舵机PID控制
    }
    else    //否则舵机默认归中
    {
        Steer_Set(SERVO_MOTOR_MID);//给舵机95度
    }
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
int view_flag = 0;
void Follow_track3(void)
{
    //科目三
    get_point_distance();       //把实时位置赋给home_run[L]，用于对比实时对比
    GPS_Two_point_error();      //GPS解析，计算方向误差
    GPS_Two_point_Distance();   //要计算两点距离，用于实现距离PID

    ips200_show_int(0, 100, next, 2);//显示实时到达点位
    //
    if(KM3weizixuanze == 0)//默认坡道
    {
        if(home_run[next] < need_distance3  && home_run[next]!=0 && next==0)
        {
            next=1;
            Speed_duty=1;  //不为0，开启舵机PID
            flag_speed = 0; //为0时开启位置PID
            Speed_PID_Init(Speed_km3_Buf[0]); //速度PID限幅
            Buzzer_SetTimer(50);
        }
        if(home_run[next] < need_distance3  && next == 1)
        {
            next = 2;
            Speed_PID_Init(Speed_km3_Buf[1]);
            Buzzer_SetTimer(50);
        }
        if(home_run[next] <need_distance3  && next == 2)
        {
            next = 3;
            Speed_PID_Init(Speed_km3_Buf[2]);
            Buzzer_SetTimer(50);
        }
        if(home_run[next] <need_distance3  && next == 3)
        {
            next = 4;
            Speed_PID_Init(Speed_km3_Buf[3]);
            Buzzer_SetTimer(50);
        }
        if(home_run[next] <need_distance3  && next == 4)
        {
            next = 5;
            Speed_PID_Init(Speed_km3_Buf[4]);
            Buzzer_SetTimer(50);
        }
        if(home_run[next] < need_distance3  && next==5)
        {
            next = 6;
            Speed_PID_Init(Speed_km3_Buf[5]);
            Buzzer_SetTimer(50);
        }

        if(home_run[next] < need_distance3  && next == 6)
        {
            next = 7;
            Speed_PID_Init(Speed_km3_Buf[6]);
            Buzzer_SetTimer(50);
        }
        if(home_run[next] <need_distance3  && next == 7)
        {
            next = 8;
            Speed_PID_Init(Speed_km3_Buf[7]);
            Buzzer_SetTimer(50);
        }
        if(home_run[next] < need_distance3 && next == 8)
        {
            view_flag = 1;
            next = 9;
            Speed_PID_Init(Speed_km3_Buf[8]);
            Buzzer_SetTimer(100);
        }
        if(home_run[next] < need_distance3 && next == 9)
        {
            view_flag = 2;
            next = 10;
            Speed_PID_Init(Speed_km3_Buf[9]);
            Buzzer_SetTimer(500);
        }
        if(home_run[next] <need_distance3  && next == 10)
        {
            view_flag = 0;
            next = 11;
            Speed_PID_Init(Speed_km3_Buf[10]);
            Buzzer_SetTimer(50);
        }
        if(home_run[next] <need_distance3  && next == 11)
        {
            view_flag = 0;
            next = 12;
            Speed_PID_Init(Speed_km3_Buf[11]);
            Buzzer_SetTimer(50);
        }
        if(home_run[next] <need_distance3  && next == 12)
        {
            next = 13;
            Speed_PID_Init(Speed_km3_Buf[12]);
            Buzzer_SetTimer(50);
        }
        if(home_run[next] <need_distance3  && next == 13)
        {
            next = 14;
            delay_ScanTemp = 0;
            dispatch_car = 0;//关闭发车标志位
            Speed_duty=0;    //关闭舵机PID
            flag_speed = 1;  //关闭位置PID，给电机速度为0
            Buzzer_SetTimer(1000);
        }
    }
    else    //桥洞
    {
        if(home_run[next] < need_distance3  && home_run[next]!=0 && next==0)
        {
            next=1;
            view_flag = 1;
            Speed_duty=1;  //不为0，开启舵机PID
            flag_speed = 0; //为0时开启位置PID
            Speed_PID_Init(Speed_km3_Buf1[0]); //速度PID限幅
            Buzzer_SetTimer(50);
        }
        if(home_run[next] < need_distance3  && next == 1)
        {
            next = 2;
            view_flag = 2;
            Speed_PID_Init(Speed_km3_Buf1[1]);
            Buzzer_SetTimer(50);
        }
        if(home_run[next] <need_distance3  && next == 2)
        {
            next = 3;
            view_flag = 0;
            Speed_PID_Init(Speed_km3_Buf1[2]);
            Buzzer_SetTimer(50);
        }
        if(home_run[next] <need_distance3  && next == 3)
        {
            next = 4;
            Speed_PID_Init(Speed_km3_Buf1[3]);
            Buzzer_SetTimer(50);
        }
        if(home_run[next] <need_distance3  && next == 4)
        {
            next = 5;
            Speed_PID_Init(Speed_km3_Buf1[4]);
            Buzzer_SetTimer(50);
        }
        if(home_run[next] < need_distance3  && next==5)
        {
            next = 6;
            Speed_PID_Init(Speed_km3_Buf1[5]);
            Buzzer_SetTimer(50);
        }

        if(home_run[next] < need_distance3  && next == 6)
        {
            next = 7;
            Speed_PID_Init(Speed_km3_Buf1[6]);
            Buzzer_SetTimer(50);
        }
        if(home_run[next] <need_distance3  && next == 7)
        {
            next = 8;
            Speed_PID_Init(Speed_km3_Buf1[7]);
            Buzzer_SetTimer(50);
        }
        if(home_run[next] < need_distance3 && next == 8)
        {
            next = 9;
            Speed_PID_Init(Speed_km3_Buf1[8]);
            Buzzer_SetTimer(100);
        }
        if(home_run[next] < need_distance3 && next == 9)
        {
            next = 10;
            Speed_PID_Init(Speed_km3_Buf1[9]);
            Buzzer_SetTimer(500);
        }
        if(home_run[next] <need_distance3  && next == 10)
        {
            next = 11;
            Speed_PID_Init(Speed_km3_Buf1[10]);
            Buzzer_SetTimer(50);
        }
        if(home_run[next] <need_distance3  && next == 11)
        {
            view_flag = 0;
            next = 12;
            Speed_PID_Init(Speed_km3_Buf1[11]);
            Buzzer_SetTimer(50);
        }
        if(home_run[next] <need_distance3  && next == 12)
        {
            next = 13;
            Speed_PID_Init(Speed_km3_Buf1[12]);
            Buzzer_SetTimer(50);
        }
        if(home_run[next] <need_distance3  && next == 13)
        {
            next = 14;
            delay_ScanTemp = 0;
            dispatch_car = 0;//关闭发车标志位
            Speed_duty=0;    //关闭舵机PID
            flag_speed = 1;  //关闭位置PID，给电机速度为0
            Buzzer_SetTimer(1000);
        }
    }

    //电机速度模式选择
    if(flag_speed == 0 && dispatch_car ==1) //flag_speed如果是0，开启PID
    {
        Pwm_Motor(resultSpeed);      //电机速度赋值
    }
    else
    {
        Pwm_Motor(0);   //否则速度为0
    }

    //舵机PID选择是否开启
    if(Speed_duty != 0) //如果速度不为0，进行舵机PID
    {
        Control_straight(); //舵机PID控制
    }
    else    //否则舵机默认归中
    {
        Steer_Set(SERVO_MOTOR_MID);//给舵机95度
    }

//    if(Speed_duty != 0 && view_flag != 2) //如果速度不为0，进行舵机PID
//    {
//        Control_straight(); //舵机PID控制
//    }
//    if(Speed_duty == 0 && view_flag != 2)    //否则舵机默认归中
//    {
//        Steer_Set(SERVO_MOTOR_MID);//给舵机95度
//    }
//    if(view_flag == 1 || view_flag == 2)
//    {
//        //开启摄像头
//        ImageProcess();
//        ips200_show_gray_image(0, 0, display_image[0], LCDW, LCDH, MT9V03X_W, MT9V03X_H,0);
//    }
}

/************************************************
 *  函数名称: 按键调速：KM_Speed_Change(uint8 KM)
 *  功能说明：
 *  参数说明：
 *  参   数：科目
 *  函数返回：无
 *  修改时间：2025年7月17日
 *  备   注：
 ************************************************/
uint8 j;
void KM_Speed_Change(uint8 KM)
{
    switch (KM)
    {
        case 1:    //科目一
        {
            ips200_show_string(195, 16*6, "sd1"); // 科目一调速标志
            if(key1_flag == 1)
            {
                key1_flag=0;
                Buzzer_SetTimer(50);
                j++;
                if(j>7){j=0;}
            }
            if(key2_flag == 1)
            {
                key2_flag=0;
                Buzzer_SetTimer(50);
                Speed_KM1_Buf[j]++;
            }
            if(key3_flag == 1)
            {
                key3_flag=0;
                Buzzer_SetTimer(50);
                Speed_KM1_Buf[j]--;
            }
            ips200_show_string(0, 16*18, "Nex");ips200_show_uint( 30, 16*18,j+1 , 3);ips200_show_uint( 60, 16*18,Speed_KM1_Buf[j] , 3);  //
        }
            break;
        case 2:     //科目二
        {
            ips200_show_string(195, 16*6, "sd2"); //科目二调速标志

        }
            break;
        case 3:     //科目三
        {
            ips200_show_string(195, 16*6, "sd3"); //科目三调速标志

        }
            break;
    }
}

