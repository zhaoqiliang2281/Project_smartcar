/*
 * project3.c
 *
 *  Created on: 2026年5月26日
 *      Author: 耿子卓
 */
#include "zf_common_headfile.h"

/************************************************
 *  函数名称: 科目3踩点，存点，读点,运行：System_NSetting_KM3(void)
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月26日
 *  备   注：按
 ************************************************/
uint8 Nag_Flag = 0;
uint16 P3_GPS_go_num=0;
uint16 P3_GPS_Back_num=0;

int32 P3_GPS_Read_go_num=0;
int32 P3_GPS_Read_Back_num=0;


uint8 Direction_Flag = 0;

//Key8_Num = gpio_get_level(KEY8);
uint8 P3GPS_Flag=0;
//uint32 P3_LOG[]

void System_NSetting_KM3(void)
{
    if(!gpio_get_level(sw1) &&  !gpio_get_level(sw2))//1,2上
    {
        if(key1_flag == 1)//踩点
        {
            key1_flag=0;
            Nag_Flag = 1;
            Buzzer_check(50);
        }
        if(key2_flag == 1)  //存Flash
        {
            key2_flag=0;
            Nag_Flag = 0;
            Nag_FlashWrite();
            Buzzer_check(50);
        }
        if(key3_flag == 1) //读Flash
        {
            key3_flag=0;
            Nag_Flag = 0;
            Nag_FlashRead();
            Buzzer_check(50);
        }
        if(key4_flag == 1)  //倒着走
        {
            key4_flag=0;
            Nag_ReversePath();
            Buzzer_check(50);
        }
    }
    else if(gpio_get_level(sw1) &&  gpio_get_level(sw2))//1,2下
    {
        if(key1_flag == 1)
        {
            key1_flag=0;
            Nag_Flag = 4;
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

//        Send_Flag_Get();


    }
    else if(gpio_get_level(sw1) &&  !gpio_get_level(sw2))//1下,2上
    {
        if(key1_flag == 1)
        {
            key1_flag=0;

            Buzzer_check(50);
        }
        if(key2_flag == 1)
        {
            key2_flag=0;

            Buzzer_check(50);
        }

    }
    else if(!gpio_get_level(sw1) &&  gpio_get_level(sw2))//1shang,2下
    {
        if(key3_flag == 1)
        {
            key3_flag=0;

        }
    }
}


/************************************************
 *  函数名称: 科目执行
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年5月26日
 *  备   注：按下按键1存点，按下按键2存点，按下按键3读点，
 ************************************************/
void Track_Km3(void)
{
    switch(Nag_Flag)
   {
        case 0:
            break;

        case 1:
          Nag_write();
          break;

        case 4:
           Nag_Read();
            break;

   }
}

//距离踩点
void Project3_GPS_Write(void)
{
       Read_Continue_Encoder_Left=Read_Encoder_Aggregate_Value(TIM5_ENCODER);
       delta = Read_Continue_Encoder_Left - Last_Encoder;
       Direction_Flag = gpio_get_level(KEY8);
       //1,2下
       if((abs(delta) >= GPS_Set_mileage)&& P3_GPS_go_num+P3_GPS_Back_num < P3GPS_MAX  )
       {

           key1_flag=0;
           Buzzer_check(50);
           LAT_data[P3_GPS_go_num+P3_GPS_Back_num] = gnss.latitude;         //把实时纬度给存储纬度
           LOG_data[P3_GPS_go_num+P3_GPS_Back_num] = gnss.longitude;        //把实时经度给存储经度
           if(Direction_Flag == 1)
           {
               P3_GPS_go_num++;
           }
           else if(Direction_Flag == 0)
           {
               P3_GPS_Back_num++;
           }

           Last_Encoder = Read_Continue_Encoder_Left;
       }
       else if(!gpio_get_level(sw1) && !gpio_get_level(sw2)) //1,2上
       {
           ips200_show_string(230, 16*8, "S");
           if(key1_flag == 1 && P3GPS_Flag == 1 && P3_GPS_go_num+P3_GPS_Back_num < P3GPS_MAX)
           {
               key1_flag=0;
               Buzzer_check(50);
               LAT_data[P3_GPS_go_num] = gnss.latitude;         //把实时纬度给存储纬度
               LOG_data[P3_GPS_go_num] = gnss.longitude;        //把实时经度给存储经度
               if(Direction_Flag == 1)
               {
                   P3_GPS_go_num++;
               }
               else if(Direction_Flag == 0)
               {
                   P3_GPS_Back_num++;
               }
               Last_Encoder = Read_Continue_Encoder_Left;
               Buzzer_check(50);
           }

       }

}

uint32 test[84] = {0};

//-------------------------------------------------------------------------------------------------------------------
// GPS存入Flash
//-------------------------------------------------------------------------------------------------------------------
void P3GPS_FlashWrite(void)
{
    flash_buffer_clear();

    flash_union_buffer[499].uint32_type = P3_GPS_go_num;
    flash_union_buffer[500].uint32_type = P3_GPS_Back_num;

    for(int i = 0;i<P3_GPS_go_num+P3_GPS_Back_num;i++)
    {
        flash_union_buffer[i*2].uint32_type =  LAT_data1[i];
        flash_union_buffer[(i*2)+1].uint32_type =  LOG_data1[i];
    }

    if(flash_check(0, 8))
    {
       flash_erase_page(0, 8);
    }

    flash_write_page_from_buffer(0, 8);

    ips200_show_string(0, 16*10, "P3GPS flash");
}


void reverse_array(uint32 *arr, int len) // 数组反转
{
    uint32 temp;
    for(int i = 0; i < len / 2; i++)
    {
        temp = arr[i];
        arr[i] = arr[len - 1 - i];
        arr[len - 1 - i] = temp;
    }
}





//-------------------------------------------------------------------------------------------------------------------
// GPS读取Flash
//-------------------------------------------------------------------------------------------------------------------、
void P3_GPSRead(void)
{
    if(flash_check(0, 8))
    {
        flash_read_page_to_buffer(0, 8);

        P3_GPS_Read_go_num= flash_union_buffer[499].uint32_type;
        P3_GPS_Read_Back_num = flash_union_buffer[500].uint32_type;

//        for(int i = 0;i<84;i++)
//        {
//
//            LOG_data1[i] = flash_union_buffer[i].uint32_type;
//            LAT_data1[i] = flash_union_buffer[i].uint32_type;
//            printf("%d,%d,%d\n\r",(int)LAT_data1[i],(int)LOG_data1[i],flash_union_buffer[i].uint32_type);
//        }

        for(int i = 0;i<P3_GPS_Read_go_num + P3_GPS_Read_Back_num;i++)
        {
            LAT_data1[i] = flash_union_buffer[i*2].uint32_type;
            LOG_data1[i] = flash_union_buffer[(i*2)+1].uint32_type;
        }
        reverse_array(LAT_data1,P3_GPS_Read_go_num + P3_GPS_Read_Back_num);
        reverse_array(LOG_data1,P3_GPS_Read_go_num + P3_GPS_Read_Back_num);
        ips200_show_string(172, 16*11, "OK flash");
//        for(int L = 0;L<=P3_GPS_Read_num;L++)
//        {
//            home_run[L] = get_two_points_distance(gnss.latitude, gnss.longitude, LAT_data_go[L], LOG_data_go[L]);
//        }
    }
    else
    {
        ips200_show_string(172, 16*11, "NO flash");
    }
}


void P3GPS_track1(void)
{
//    get_point_distance();       //把实时位置赋给home_run[L]，用于对比实时对比

    home_run[next] = get_two_points_distance(gnss.latitude, gnss.longitude, LAT_data_go[next], LOG_data_go[next]);

    GPS_Two_point_error();      //计算方向误差
//    GPS_Two_point_Distance();   //要计算两点距离，用于实现距离PID

    ips200_show_int(0, 100, next, 2);//显示实时到达点位


    if(home_run[next] < need_distance  && next==1)
    {
       next++;

       Back_Motor_PID_Start_flag = 1; //为1时开启位置PID

       Back_Motor_Set_Speed = 300;//设置速度
       Buzzer_SetTimer(50);
    }
    if(home_run[next] < need_distance &&  next!=1 &&  next < P3_GPS_Read_go_num)
    {
       next++;
       Back_Motor_Set_Speed = 300;//设置速度
       Buzzer_SetTimer(50);
    }

    if(home_run[next] < need_distance &&  next!=1 &&  (next > P3_GPS_Read_go_num && next<P3_GPS_Read_go_num + P3_GPS_Read_Back_num-1))
    {
       next++;
       Back_Motor_Set_Speed = -200;//设置速度
       Buzzer_SetTimer(50);
    }

      //电机速度模式选择
    if(next >= 1 && next != P3_GPS_Read_go_num + P3_GPS_Read_Back_num) //flag_speed如果是0，开启PID
    {
      Back_Motor_SetPwm(Back_Motor_Set_Speed,Back_Motor_Set_Speed);      //电机速度赋值
    }


      //方向PID选择是否开启
    if(dispatch_car == 1)
    {
      Control_straight();
    }

    if(home_run[next] <1.5  && next == P3_GPS_Read_go_num && P3_GPS_Read_Back_num !=0)
    {
        next++;
        Buzzer_SetTimer(50);
        system_delay_ms(200);

        Back_Motor_Set_Speed = -200;//设置速度
    }
    if(home_run[next] <1.5  && next >= P3_GPS_Read_go_num + P3_GPS_Read_Back_num-1)
    {
        next = P3_GPS_Read_go_num + P3_GPS_Read_Back_num;
        delay_ScanTemp = 0;//关闭定时器延迟发车标志位
        dispatch_car = 0;//关闭发车标志位
        Back_Motor_Set_Speed = 0;//设置速度
        Back_Motor_SetPwm(0,0);
        Back_Motor_PID_Start_flag = 0; //关闭位置PID，给电机速度为0
        Buzzer_SetTimer(1000);
        P3GPS_Flag=0;
    }

}

/************************************************
 *  函数名称: 科目执行
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年5月26日
 *  备   注：GPS+编码器
 ************************************************/
void GPS_Km3(void)
{
    switch(P3GPS_Flag)
   {
        case 0:
            break;

        case 1:
            Project3_GPS_Write();
          break;

        case 2:
            P3GPS_track1();
            break;

   }
}


void System_gps_KM3(void)
{
    if(!gpio_get_level(sw1) &&  !gpio_get_level(sw2))//1,2上
    {
        ips200_show_string(230, 16*8, "S");
        if(key1_flag == 1 && P3GPS_Flag == 0)
        {
            key1_flag=0;
            P3GPS_Flag=1;
            Buzzer_check(50);
        }
        if(key2_flag == 1)
        {
            key2_flag=0;
            for(int L = 0;L<P3_GPS_go_num+P3_GPS_Back_num;L++)
            {
                LAT_data1[L] = LAT_data[L]*1000000.0;   //小数整形化
                LOG_data1[L] = LOG_data[L]*1000000.0;
            }
//            Up_flash_point();
            P3GPS_FlashWrite();   //科目三（存flash）
            P3GPS_Flag=0;
            Buzzer_check(50);
        }
        if(key3_flag == 1)
        {
            key3_flag=0;
            P3_GPSRead();   //科目三(读取flash)
//            GPS_Remote();//gps除零漂
//            Read_flash_point();
            for(int L = 0;L<P3_GPS_Read_go_num + P3_GPS_Read_Back_num;L++)
            {
                LAT_data_go[L] = LAT_data1[L]/1000000.0;    //整形小数化
                LOG_data_go[L] = LOG_data1[L]/1000000.0;
//                printf("%d,%d,%d\n\r",(int)LAT_data1[L],(int)LOG_data1[L],flash_union_buffer[L].uint32_type);
            }
            P3GPS_Flag=0;
            Buzzer_check(50);
        }
        if(key4_flag == 1)
        {
            key4_flag=0;
            P3GPS_Flag=2;
            dispatch_car= 1;
            next =1;
            Back_Motor_Set_Speed=300;
            Back_Motor_SetPwm(300,300);
            Buzzer_check(50);
        }
    }
    else if(gpio_get_level(sw1) &&  gpio_get_level(sw2))//1,2下
    {
        ips200_show_string(230, 16*8, "X");
        if(key1_flag == 1)
        {
            key1_flag=0;
//            next = 1;
            gps_point gps_point_test[P3_GPS_Read_go_num + P3_GPS_Read_Back_num];
            for(int i = 0; i < P3_GPS_Read_go_num + P3_GPS_Read_Back_num; i++) {
                gps_point_test[i].lat = LAT_data_go[i];
                gps_point_test[i].lon = LOG_data_go[i];
            }
            user_gps_display_init(IPS200_TYPE,10,10,220,300);
            user_gps_transition(gps_point_test,sizeof(gps_point_test) / sizeof(gps_point_test[0]));
            user_gps_display(RGB565_PURPLE);

            Buzzer_check(50);
        }
        if(key2_flag == 1)
        {
            Buzzer_SetTimer(50);
            key2_flag=0;
            if(flash_check(0, 8))
            {
                flash_read_page_to_buffer(0, 8);

                P3_GPS_Read_go_num= flash_union_buffer[499].uint32_type;
                P3_GPS_Read_Back_num = flash_union_buffer[500].uint32_type;
                for(int i = 0;i<P3_GPS_Read_go_num + P3_GPS_Read_Back_num;i++)
                {
                    LAT_data1[i] = flash_union_buffer[i*2].uint32_type;
                    LOG_data1[i] = flash_union_buffer[(i*2)+1].uint32_type;
                }
                ips200_show_string(172, 16*11, "OK flash");
            }
            else
            {
                ips200_show_string(172, 16*11, "NO flash");
            }
            for(int L = 0;L<P3_GPS_Read_go_num + P3_GPS_Read_Back_num;L++)
            {
                LAT_data_go[L] = LAT_data1[L]/1000000.0;    //整形小数化
                LOG_data_go[L] = LOG_data1[L]/1000000.0;
//                printf("%d,%d,%d\n\r",(int)LAT_data1[L],(int)LOG_data1[L],flash_union_buffer[L].uint32_type);
            }
            P3GPS_Flag=0;

        }

    }
    else if(gpio_get_level(sw1) &&  !gpio_get_level(sw2))//1下,2上
    {
//        if(key1_flag == 1)
//        {
//            key1_flag=0;
//        }
//        if(key2_flag == 1)
//        {
//            key2_flag=0;
//        }

    }
    else if(!gpio_get_level(sw1) &&  gpio_get_level(sw2))//1shang,2下
    {
        if(key3_flag == 1)
        {
            key3_flag=0;
            Buzzer_SetTimer(50);
            LAT_error=gnss.latitude-LAT_data_go[0];
            LOG_error=gnss.longitude-LOG_data_go[0];
            for(int i=0;i<P3_GPS_Read_go_num + P3_GPS_Read_Back_num;i++)
            {
                LAT_data_go[i]+=LAT_error;
                LOG_data_go[i]+=LOG_error;
            }
        }
    }
}
