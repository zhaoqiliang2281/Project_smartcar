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
float S_Yaw_Target = 0;         // 插值后的目标Yaw
float S_Last_Yaw_Target = 0;    // 上一次目标Yaw
int32 S_Last_Encoder_Read=0;
int32 S_Read_Continue_Encoder_Left=0;
int32 S_delta2;

#define S_YAW_MAX_CHANGE   20.0f // 最大Yaw变化限制

float S_start_yaw;


/************************************************
 *  函数名称: 科目2踩点，存点，读点设置：System_NSetting_KM2(void)
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月26日
 *  备   注：按下按键1采点，按下按键2存点，按下按键3读点，
 ************************************************/
void System_NSetting_KM2(void)
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
                {LAT_data_go[12],LOG_data_go[12]},
                {LAT_data_go[13],LOG_data_go[13]},//科二14个点
                {LAT_data_go[14],LOG_data_go[14]},//科二15个点
                {LAT_data_go[15],LOG_data_go[15]},//科二16个点
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
            FLASH_Get_KM2();   //科目二修改完点位后再次存入（存flash）

            Buzzer_check(50);
        }
        if(key4_flag==1)
        {
                key4_flag=0;
                Buzzer_SetTimer(50);
//                Key7_Num=0;

        }

    }
    else if(gpio_get_level(sw1) &&  !gpio_get_level(sw2))//1下,2上
    {

        Key_Change_LAT_LON();//GPS修点

    }
    else if(!gpio_get_level(sw1) &&  gpio_get_level(sw2))//1shang,2下
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

            Sport_sum_flag=3;
            Flash_sum_flag=2;
            Voice_sum_flag=1;
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
 *  函数名称: 科目执行
 *  功能说明：
 *  参数说明：Task：判断去的时候走哪一个桥洞。
 *            Task2：判断回来的时候走哪个桥洞
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月26日
 *  备   注：按下按键1存点，按下按键2存点，按下按键3读点，
 ************************************************/
uint8 Track2_Km2(uint8 Task,uint8 Task2)
{

    static uint8 sport_finish=0;
    get_point_distance();       //把实时位置赋给home_run[L]，用于对比实时对比
    GPS_Two_point_error();      //计算方向误差
    GPS_Two_point_Distance();   //要计算两点距离，用于实现距离PID

    ips200_show_int(0, 100, next, 2);//显示实时到达点位

    switch(Task)
    {
        case 0x15: //门洞1左侧
            MenDong_Flag=1;
            break;
        case 0x16: //门洞1
            MenDong_Flag=3;
            break;
        case 0x17: //门洞2
            MenDong_Flag=5;
            break;
        case 0x18: //门洞3
            MenDong_Flag=7;
            break;
        case 0x19: //门洞3右侧
            MenDong_Flag=9;
            break;
    }
    switch(Task2)
    {
        case 0x1A: //门洞1左侧
            MenDong_Flag2=1;
            break;
        case 0x1B: //门洞1
            MenDong_Flag2=3;
            break;
        case 0x1C: //门洞2
            MenDong_Flag2=5;
            break;
        case 0x1D: //门洞3
            MenDong_Flag2=7;
            break;
        case 0x1E: //门洞3右侧
            MenDong_Flag2=9;
            break;
    }

    if(home_run[next] < need_distance  && home_run[0]!=0 && next==1 && Go_Flag==1)
       {
           next=MenDong_Flag+1;

           Back_Motor_PID_Start_flag = 1; //为1时开启位置PID

           Back_Motor_Set_Speed = Speed_KM2_Buf[0];//设置速度
           Buzzer_SetTimer(50);
       }
       if(home_run[next] < need_distance  && next == MenDong_Flag+1 && Go_Flag==1)
       {
           next = MenDong_Flag+1+1;
           Back_Motor_Set_Speed = Speed_KM2_Buf[1];//设置速度
           Buzzer_SetTimer(50);
       }
        if(home_run[next] <need_distance  && next == MenDong_Flag+1+1 && Go_Flag==1)
        {
            next = 11+1;
            Back_Motor_Set_Speed = Speed_KM2_Buf[2];//设置速度
            Buzzer_SetTimer(50);
        }
        if(home_run[next] <need_distance  && next == 11+1 && Go_Flag==1)
        {
            next = 12+1;
            Back_Motor_Set_Speed = Speed_KM2_Buf[2];//设置速度
            Buzzer_SetTimer(50);
        }
        if(home_run[next] <need_distance  && next == 12+1 && Go_Flag==1)
        {
            next = MenDong_Flag2+1+1;
            Go_Flag=0;
            delay_ScanTemp = 0;//关闭定时器延迟发车标志位
            dispatch_car = 0;//关闭发车标志位
            Back_Motor_Set_Speed = Speed_KM2_Buf[7];//设置速度

            Back_Motor_PID_Start_flag = 0; //关闭位置PID，给电机速度为0
            Buzzer_SetTimer(1000);
        }
          //电机速度模式选择
        if((Go_Flag==1 || Back_Flag==1) && next > 1 ) //flag_speed如果是0，开启PID
        {
          Back_Motor_SetPwm(Back_Motor_Set_Speed,Back_Motor_Set_Speed);      //电机速度赋值
        }


          //方向PID选择是否开启
        if(dispatch_car == 1)
        {
          Control_straight();
        }
        if(IMU_PID == 1)
        {
            IMU_Control(Go_Yaw-Yaw);
        }


          //下面写在运动区的运动代码
        if(Go_Flag==0 && Back_Flag==0)
        {

            sport_finish=Sport_choise(data_store,Sport_sum_flag,Flash_sum_flag+2);
            if(!sport_finish)
            {
                Back_Flag=1;
                dispatch_car = 1;
                Back_Motor_PID_Start_flag=1;
                Back_Motor_Set_Speed=200;
            }
//

        }
          //***********************


        if(home_run[next] < need_distance  && next == MenDong_Flag2+1+1 && Back_Flag==1)
        {
            next = MenDong_Flag2+1;
            Back_Motor_Set_Speed = Speed_KM2_Buf[next-1];//设置速度
            Buzzer_SetTimer(50);
        }
        if(home_run[next] < need_distance  && next == MenDong_Flag2+1 && Back_Flag==1)
        {
           next = 13+1;
           Back_Motor_Set_Speed = Speed_KM2_Buf[next-1];//设置速度
           Buzzer_SetTimer(50);
        }
        if(home_run[next] < need_distance  && next == 13+1 && Back_Flag==1)
       {
          next = 14+1;
          Back_Motor_Set_Speed = Speed_KM2_Buf[next-1];//设置速度
          Buzzer_SetTimer(50);
       }
        if(home_run[next] <0.8  && next == 14+1 && Back_Flag==1)
        {
            next = 15+1;
            delay_ScanTemp = 0;//关闭定时器延迟发车标志位
            dispatch_car = 0;//关闭发车标志位
            Back_Motor_Set_Speed = 0;//设置速度
            Back_Motor_SetPwm(0,0);
            Back_Motor_PID_Start_flag = 0; //关闭位置PID，给电机速度为0
            Buzzer_SetTimer(1000);
            return 0;
        }

        return 1;
}


/************************************************
 *  函数名称: 纯角度pid
 *  功能说明：
 *  参数说明：Expectation_Yaw：期望的角度。
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月26日
 *  备   注：按下按键1存点，按下按键2存点，按下按键3读点，
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

uint8 Go_straight(void)//直线前进10m
{
    static int32 first_encoder = 0;
    static int32 Moving_Distance=0;//行驶距离
    if(first_run==1)
    {
        Go_Yaw = Yaw;
        Angle_Target_Position = 0;
        first_encoder=Read_Encoder_Aggregate_Value(TIM5_ENCODER);
        first_run=0;
    }

    IMU_PID=1;
    Back_Motor_SetPwm(300,300);
    Moving_Distance = Read_Encoder_Aggregate_Value(TIM5_ENCODER) - first_encoder;
    if(Moving_Distance>13150)//十米
    {
        Buzzer_check(50);
        Back_Motor_SetPwm(0,0);
        IMU_PID=0;
        first_run=1;
        system_delay_ms(500);
        return 0;
    }
    return 1;
}

uint8 Back_straight(void)//直线后退10m
{
    static int32 first_encoder = 0;
    static int32 Moving_Distance=0;//行驶距离
    if(first_run==1)
    {
        Go_Yaw = Yaw;
        Angle_Target_Position = 0;
        first_encoder=Read_Encoder_Aggregate_Value(TIM5_ENCODER);
        first_run=0;
    }
    IMU_PID=1;
    Back_Motor_SetPwm(-300,-300);
    Moving_Distance = Read_Encoder_Aggregate_Value(TIM5_ENCODER) - first_encoder;
    if(Moving_Distance<-13150)
    {
        Buzzer_check(50);
        Back_Motor_SetPwm(0,0);
        IMU_PID=0;
        first_run=1;
        system_delay_ms(500);
        return 0;
    }
    return 1;
}

uint8 Left_straight(void)//左转
{
    if(first_run==1)
    {
        Go_Yaw = (Yaw - 90 < 0) ? ((Yaw - 90) + 360) : (Yaw - 90);
        first_run=0;
        Angle_Target_Position = -900;
    }
    Back_Motor_SetPwm(300,300);
    if(abs(Get_Yaw_Error(Yaw,Go_Yaw)<=2))
    {
        Buzzer_check(50);
        Back_Motor_SetPwm(0,0);
        IMU_PID=0;
        Angle_Target_Position = 0;
        first_run=1;
        system_delay_ms(500);
        return 0;
    }
    return 1;
}

uint8 Right_straight(void)//右转
{
    if(first_run==1)
    {
        Go_Yaw = (Yaw + 90 >= 360) ? (Yaw + 90 - 360) : (Yaw + 90);
        first_run=0;
        Angle_Target_Position = 900;
    }
    Back_Motor_SetPwm(300,300);
    if(abs(Get_Yaw_Error(Yaw,Go_Yaw)) <= 2)
    {
        Back_Motor_SetPwm(0,0);
        IMU_PID=0;
        Buzzer_check(50);
        Angle_Target_Position = 0;
        first_run=1;
        system_delay_ms(500);
        return 0;
    }
    return 1;
}

uint8 Left_circlet(void)//左转一圈
{

        static int8 Yaw_Flag=1;
        if(first_run==1)
        {
            Go_Yaw=Yaw;
            first_run=0;
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
            Back_Motor_SetPwm(0,0);
            Buzzer_check(50);
            Angle_Target_Position = 0;
            first_run=1;
            system_delay_ms(500);
            return 0;
        }
        return 1;


}

uint8 Right_circlet(void)//右转一圈
{
    static int8 Yaw_Flag=1;
    if(first_run==1)
    {
        first_run=0;
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
        S_start_yaw = Yaw;  // 先获取当前实际角度作为偏移基准

        S_Yaw_Start = S_Sport_yaw[0] + S_start_yaw;
        // 终点Yaw也要加偏移
        S_Yaw_End = S_Sport_yaw[1] + S_start_yaw;

        // 归一化到0~360
        if(S_Yaw_Start >= 360.0f) S_Yaw_Start -= 360.0f;
        if(S_Yaw_Start < 0.0f) S_Yaw_Start += 360.0f;
        if(S_Yaw_End >= 360.0f) S_Yaw_End -= 360.0f;
        if(S_Yaw_End < 0.0f) S_Yaw_End += 360.0f;

        S_Yaw_Target = S_Yaw_Start;

        S_Last_Yaw_Target = S_Yaw_Start;

        S_Run_Index = 1;
        S_Last_Encoder_Read = Read_Encoder_Aggregate_Value(TIM5_ENCODER);
        S_flag = 1;

    }

    //================================================//

    S_Read_Continue_Encoder_Left = Read_Encoder_Aggregate_Value(TIM5_ENCODER);
    // 当前段行驶距离
    S_delta2 = S_Read_Continue_Encoder_Left - S_Last_Encoder_Read;

    //==================== 插值 ====================//
    // 当前段完成比例
    t = (float)abs(S_delta2) / (float)Nag_Set_mileage;

    if(t > 1.0f)
    {
        t = 1.0f;
    }

    // 计算最短角度差
    yaw_diff = angle_diff(S_Yaw_End, S_Yaw_Start);

    // 按最短方向插值
    S_Yaw_Target = S_Yaw_Start + yaw_diff * t;

    // 限制到0~360
    if(S_Yaw_Target >= 360.0f)
    {
        S_Yaw_Target -= 360.0f;
    }

    if(S_Yaw_Target < 0.0f)
    {
        S_Yaw_Target += 360.0f;
    }

    //==================== 环形限幅 ====================//

    // 注意：
    // 必须用 angle_diff
    // 不能直接减
    yaw_change = angle_diff(S_Yaw_Target, S_Last_Yaw_Target);
    // 限制每次变化量

    if(yaw_change > S_YAW_MAX_CHANGE)
    {
        yaw_change = S_YAW_MAX_CHANGE;
    }

    if(yaw_change < -S_YAW_MAX_CHANGE)
    {
        yaw_change = -S_YAW_MAX_CHANGE;
    }
    // 重新生成Yaw_Target

    S_Yaw_Target = S_Last_Yaw_Target + yaw_change;
    // 限制到0~360

    if(S_Yaw_Target >= 360.0f)
    {
        S_Yaw_Target -= 360.0f;
    }

    if(S_Yaw_Target < 0.0f)
    {
        S_Yaw_Target += 360.0f;
    }

    // 保存
    S_Last_Yaw_Target = S_Yaw_Target;

    // 前进
    Angle_Target_Position = -GPS_IMU_PID_Start(Open, angle_diff(S_Yaw_Target, Yaw), 0);
    Back_Motor_SetPwm(200, 200);

    //==================== 切换下一段 ====================//

    if(abs(S_delta2) >= Nag_Set_mileage)
    {
        S_Run_Index++;
        // 已结束
        if(S_Run_Index >= 25)
        {
            Back_Motor_SetPwm(0,0);
            Angle_Target_Position = 0;
            S_flag = 0;
            system_delay_ms(500);
            return 0;
        }

          // 当前终点变下一段起点
        S_Yaw_Start = S_Yaw_End;

        // 下一段终点（也要加偏移）
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
        S_start_yaw = Yaw;  // 先获取当前实际角度作为偏移基准

        S_Yaw_Start = SBack_Sport_yaw[24] + S_start_yaw;
        // 终点Yaw也要加偏移
        S_Yaw_End = SBack_Sport_yaw[23] + S_start_yaw;

        // 归一化到0~360
        if(S_Yaw_Start >= 360.0f) S_Yaw_Start -= 360.0f;
        if(S_Yaw_Start < 0.0f) S_Yaw_Start += 360.0f;
        if(S_Yaw_End >= 360.0f) S_Yaw_End -= 360.0f;
        if(S_Yaw_End < 0.0f) S_Yaw_End += 360.0f;

        S_Yaw_Target = S_Yaw_Start;

        S_Last_Yaw_Target = S_Yaw_Start;

        S_Run_Index = 23;
        S_Last_Encoder_Read = Read_Encoder_Aggregate_Value(TIM5_ENCODER);
        S_flag = 1;

    }

    //================================================//

    S_Read_Continue_Encoder_Left = Read_Encoder_Aggregate_Value(TIM5_ENCODER);
    // 当前段行驶距离
    S_delta2 = S_Read_Continue_Encoder_Left - S_Last_Encoder_Read;

    //==================== 插值 ====================//
    // 当前段完成比例
    t = (float)abs(S_delta2) / (float)Nag_Set_mileage;

    if(t > 1.0f)
    {
        t = 1.0f;
    }

    // 计算最短角度差
    yaw_diff = angle_diff(S_Yaw_End, S_Yaw_Start);

    // 按最短方向插值
    S_Yaw_Target = S_Yaw_Start + yaw_diff * t;

    // 限制到0~360
    if(S_Yaw_Target >= 360.0f)
    {
        S_Yaw_Target -= 360.0f;
    }

    if(S_Yaw_Target < 0.0f)
    {
        S_Yaw_Target += 360.0f;
    }

    //==================== 环形限幅 ====================//

    // 注意：
    // 必须用 angle_diff
    // 不能直接减
    yaw_change = angle_diff(S_Yaw_Target, S_Last_Yaw_Target);
    // 限制每次变化量

    if(yaw_change > S_YAW_MAX_CHANGE)
    {
        yaw_change = S_YAW_MAX_CHANGE;
    }

    if(yaw_change < -S_YAW_MAX_CHANGE)
    {
        yaw_change = -S_YAW_MAX_CHANGE;
    }
    // 重新生成Yaw_Target

    S_Yaw_Target = S_Last_Yaw_Target + yaw_change;
    // 限制到0~360

    if(S_Yaw_Target >= 360.0f)
    {
        S_Yaw_Target -= 360.0f;
    }

    if(S_Yaw_Target < 0.0f)
    {
        S_Yaw_Target += 360.0f;
    }

    // 保存
    S_Last_Yaw_Target = S_Yaw_Target;

    // 前进
    Angle_Target_Position = -GPS_IMU_PID_Start(Open, angle_diff(S_Yaw_Target, Yaw), 0);
    Back_Motor_SetPwm(-200, -200);

    //==================== 切换下一段 ====================//

    if(abs(S_delta2) >= Nag_Set_mileage)
    {
        S_Run_Index--;
        // 已结束
        if(S_Run_Index <= 0)
        {
            Back_Motor_SetPwm(0,0);
            Angle_Target_Position = 0;
            S_flag = 0;
            system_delay_ms(500);
            return 0;
        }

          // 当前终点变下一段起点
        S_Yaw_Start = S_Yaw_End;

        // 下一段终点（也要加偏移）
        S_Yaw_End = SBack_Sport_yaw[S_Run_Index] + S_start_yaw;

        // 归一化
        if(S_Yaw_End >= 360.0f) S_Yaw_End -= 360.0f;
        if(S_Yaw_End < 0.0f) S_Yaw_End += 360.0f;

        S_Last_Encoder_Read = S_Read_Continue_Encoder_Left;
    }
    return 1;
}

/************************************************
 *  函数名称: 运动代码
 *  功能说明：
 *  参数说明：sport_sum：需要做多少个运动（0-8）。
 *         sprot_num[]:数组存放运动的标志位。
 *         first_num：动作在数组中起始的位置
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年5月21日
 *  备   注：
 ************************************************/
uint8 Sport_choise(uint8 sprot_num[],uint8 sport_sum,uint8 first_num)
{
    static uint8 sport_flag = 0;
    static uint8 sport_continue;
    static uint8 is_initialized = 0;  // 标记是否已初始化
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

            case 0x21:             //蛇形前进
                sport_continue=Go_S();
                if(sport_continue ==0)
                    sport_flag++;
                break;

            case 0x22:             //蛇形后退
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
 *  函数名称: 灯光任务
 *  功能说明：
 *  参数说明：Flash_sum：需要做多少个灯光任务（0-8）。
 *         Flash_num[]:数组存放运动的标志位。
 *         first_num：任务在数组中起始的位置
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年5月26日
 *  备   注：
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
 *  函数名称: 鸣笛任务
 *  功能说明：
 *  参数说明：Voice_num：需要做多少个鸣笛任务（0-9）。
 *         Voice_sum[]:数组存放运动的标志位。
 *         first_num：任务在数组中起始的位置
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年5月26日
 *  备   注：
 ************************************************/
uint8 Voice_choise(uint8 Voice_num[],uint8 Voice_sum,uint8 first_num)
{
    for(int i =first_num;i < first_num+Voice_sum;i++)
    {
        execute_command(Voice_num[i]);
        if(i==first_num+Voice_sum-1)
            return 0;
    }
    return 1;
}




// ==================== 跨核任务接口(灯光/语音交由 CPU1 执行) ====================
/************************************************
 *  函数名称: cmd_task_submit 语音命令跨核提交
 *  功能说明：把“本次新增的语音命令”提交给 CPU1 逐条执行,非阻塞
 *  参数说明：cmd: 命令数组(建议只含本次新增命令); sum: 命令条数
 *  函数返回：1=已提交给CPU1; 0=CPU1正在忙(上一批未完成)或命令数为0
 *  备   注：先写数据再置 ready,CPU1 执行完置 done
 ************************************************/
uint8 cmd_task_submit(uint8 cmd[], uint32 sum)
{
    uint32 i;
    if (sum == 0) return 0;                                 // 没有命令
    if (!mbox_done) return 0;                               // CPU1 还在执行上一批
    mbox_done = 0;
    for (i = 0; i < sum; i++)                               // 拷贝到共享区,避免直接读 CPU0 数组
    {
        mbox_cmd[i] = cmd[i];
    }
    mbox_sum = sum;
    __dsync();                                              // 数据同步屏障:确保数据写完后再发布 ready
    mbox_ready = 1;
    return 1;
}


