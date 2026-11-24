/*
 * navigation.c
 *
 *  Created on: 2024年10月16日
 *      Author: Monst
 *
 */

#include "zf_common_headfile.h"

int32 Last_Encoder=0;
int32 Nag_Point_Flag=0;
int32 Nag_ResdPoint_Flag=0;

int32 Nag_data[NagPoint_MAX] = {0};
int32 Nag_data2[NagPoint_MAX] = {0};

int32 delta;

int32 Last_Encoder_Read=0;
int32 delta2;

int32 Run_Index = 0;


float Yaw_Start = 0;          // 当前段起始Yaw
float Yaw_End = 0;            // 当前段结束Yaw
float Yaw_Target = 0;         // 插值后的目标Yaw
float Last_Yaw_Target = 0;    // 上一次目标Yaw

#define YAW_MAX_CHANGE   20.0f // 最大Yaw变化限制

////===================0======================////

int8  Nag_Target_Dir = 1;
int8  Last_Dir = 0;           // 上一次移动方向

int32 Read_Continue_Encoder_Left=0;


//-------------------------------------------------------------------------------------------------------------------
// 惯导采集路线
//-------------------------------------------------------------------------------------------------------------------
void Nag_write(void)
{
    int32 pack;
    int8 dir;

    Read_Continue_Encoder_Left=Read_Encoder_Aggregate_Value(TIM5_ENCODER);//左侧编码器
    delta = Read_Continue_Encoder_Left - Last_Encoder;

    if((abs(delta) >= Nag_Set_mileage) && Nag_Point_Flag < NagPoint_MAX)
    {
        dir = (delta > 0) ? 1 : -1;

        pack = (dir << 24) | (int32)(Yaw*100);

        Nag_data[Nag_Point_Flag++] = pack;

        Last_Encoder = Read_Continue_Encoder_Left;
        Buzzer_check(50);
    }
}


//-------------------------------------------------------------------------------------------------------------------
// 惯导路线存入Flash
//-------------------------------------------------------------------------------------------------------------------
void Nag_FlashWrite(void)
{
    flash_buffer_clear();

    flash_union_buffer[0].uint32_type = Nag_Point_Flag;

    for(int i = 1;i<=Nag_Point_Flag;i++)
    {
        flash_union_buffer[i].uint32_type =  Nag_data[i-1];
    }

    if(flash_check(0, 9))
    {
       flash_erase_page(0, 9);
    }

    flash_write_page_from_buffer(0, 9);

    ips200_show_string(0, 16*10, "Nag flash");
}


//-------------------------------------------------------------------------------------------------------------------
// 惯导路线Flash读取
//-------------------------------------------------------------------------------------------------------------------
void Nag_FlashRead(void)
{
    if(flash_check(0, 9))
    {
        flash_read_page_to_buffer(0, 9);

        Nag_ResdPoint_Flag= flash_union_buffer[0].uint32_type;

        for(int i = 1;i<=Nag_ResdPoint_Flag;i++)
        {
            Nag_data2[i-1] = flash_union_buffer[i].uint32_type;
        }

        ips200_show_string(172, 16*11, "OK flash");
    }
    else
    {
        ips200_show_string(172, 16*11, "NO flash");
    }
}


//-------------------------------------------------------------------------------------------------------------------
// 数组数据解析
//-------------------------------------------------------------------------------------------------------------------
void Nag_Decode(int32 raw, float *yaw, int8 *dir)
{
    *yaw = (raw & 0x00FFFFFF) / 100.0f;

    *dir = (int8)(raw >> 24);

    if(*dir != 1 && *dir != -1)
    {
        *dir = 1;
    }
}


//-------------------------------------------------------------------------------------------------------------------
// 惯导路线复现（插值平滑版）
//-------------------------------------------------------------------------------------------------------------------
void Nag_Read(void)
{
    static uint8 Dir_flag=0;
    float t;
   float yaw_diff;
   float yaw_change;

   //==================== 初始化 ====================//

   if(Run_Index == 0 && Nag_ResdPoint_Flag > 1)
   {
       // 起点Yaw
       Nag_Decode(Nag_data2[0],&Yaw_Start,&Nag_Target_Dir);

       // 终点Yaw
       Nag_Decode(Nag_data2[1],&Yaw_End,&Nag_Target_Dir);
       Yaw_Target = Yaw_Start;
       Last_Yaw_Target = Yaw_Start;
       Last_Dir = 0;
       Run_Index = 1;
       Last_Encoder_Read =Read_Encoder_Aggregate_Value(TIM5_ENCODER);

   }

   //================================================//

   Read_Continue_Encoder_Left=Read_Encoder_Aggregate_Value(TIM5_ENCODER);
   // 当前段行驶距离
   delta2 =Read_Continue_Encoder_Left - Last_Encoder_Read;

   //==================== 插值 ====================//
   // 当前段完成比例
   t = (float)abs(delta2) / (float)Nag_Set_mileage;

   if(t > 1.0f)
   {
       t = 1.0f;
   }

   // 计算最短角度差
   yaw_diff =angle_diff(Yaw_End,Yaw_Start);

   // 按最短方向插值
   Yaw_Target = Yaw_Start + yaw_diff * t;

   // 限制到0~360

   if(Yaw_Target >= 360.0f)
   {
       Yaw_Target -= 360.0f;
   }

   if(Yaw_Target < 0.0f)
   {
       Yaw_Target += 360.0f;
   }

   //==================== 环形限幅 ====================//

   // 注意：
   // 必须用 angle_diff
   // 不能直接减
   yaw_change = angle_diff(Yaw_Target,Last_Yaw_Target);
   // 限制每次变化量

   if(yaw_change > YAW_MAX_CHANGE)
   {
       yaw_change = YAW_MAX_CHANGE;
   }

   if(yaw_change < -YAW_MAX_CHANGE)
   {
       yaw_change = -YAW_MAX_CHANGE;
   }
   // 重新生成Yaw_Target

   Yaw_Target = Last_Yaw_Target + yaw_change;
   // 限制到0~360

   if(Yaw_Target >= 360.0f)
   {
       Yaw_Target -= 360.0f;
   }

   if(Yaw_Target < 0.0f)
   {
       Yaw_Target += 360.0f;
   }

   // 保存
   Last_Yaw_Target = Yaw_Target;

   //================================================//

   if((Last_Dir * Nag_Target_Dir) == -1 && !Dir_flag)
   {
       Dir_flag=1;
       Back_Motor_SetPwm(0,0);
       system_delay_ms(1000);
   }
   //==================== 控制 ====================//
   if(Nag_Target_Dir == 1)
   {
       // 前进
       Angle_Target_Position = -GPS_IMU_PID_Start(Open,angle_diff(Yaw_Target,Yaw),0,Nag_Target_Dir);
       Back_Motor_SetPwm_Diff(300, angle_diff(Yaw_Target, Yaw));      // 差速转向
   }
   else
   {
       // 后退
       Angle_Target_Position = GPS_IMU_PID_Start(Open,angle_diff(Yaw_Target,Yaw),0,Nag_Target_Dir);
       Back_Motor_SetPwm_Diff(-300, -angle_diff(Yaw_Target, Yaw));     // 倒车差速(误差取反)
   }


   //==================== 切换下一段 ====================//

   if(abs(delta2) >= Nag_Set_mileage)
   {
       Run_Index++;

       Last_Dir = Nag_Target_Dir;
       // 已结束
       if(Run_Index >= Nag_ResdPoint_Flag - 1)
       {
           Back_Motor_SetPwm(0,0);
           Angle_Target_Position = 0;
           return;
       }

       // 当前终点变下一段起点
       Yaw_Start = Yaw_End;

       // 下一段终点
       Nag_Decode(Nag_data2[Run_Index],&Yaw_End,&Nag_Target_Dir);
       Last_Encoder_Read = Read_Continue_Encoder_Left;
   }

       //================================================//
}


void Nag_ReversePath(void)
{
    int32 reversed_data[NagPoint_MAX];
    float yaw;
    int8 dir;
    float reversed_yaw;
    int8 reversed_dir;

    if(Nag_ResdPoint_Flag == 0) return;

    for(int i = 0; i < Nag_ResdPoint_Flag; i++)
    {
        // 取正向的第 (总点数-1-i) 个点
        uint32 src_index = Nag_ResdPoint_Flag - 1 - i;
        Nag_Decode(Nag_data2[src_index], &yaw, &dir);

        // 角度转180°
        reversed_yaw = yaw + 180.0f;
        if(reversed_yaw >= 360.0f) reversed_yaw -= 360.0f;

        // 方向保持前进（dir已经是1，因为正着走）
        reversed_dir = 1;  // 始终前进

        // 重新打包
        reversed_data[i] = (reversed_dir << 24) | (int32)(reversed_yaw * 100);
    }

    // 覆盖原数组
    for(int i = 0; i < Nag_ResdPoint_Flag; i++)
    {
        Nag_data2[i] = reversed_data[i];
    }
}


/************************************************
 *  备   注：  以下为从第二次校赛之后新方案，用编码器及陀螺仪构建一个二维坐标，GPS则定时进行校准
 ************************************************/



/*******************************************************************

int32 path[PATH_MAX];    // 路径存储数组
uint16 path_num = 0;     // 当前已经记录的点数

float pos_x = 0;      // 东向坐标(m)
float pos_y = 0;      // 北向坐标(m)

float last_yaw = 0.0f;// 上一次Yaw


int32 Pack_XY(int16 x_cm, int16 y_cm)  //打包数据函数
{
    // 将X左移16位放入高16位
    // Y直接放入低16位

    return ((int32)x_cm << 16)
            | ((uint16)y_cm);
}


void Unpack_XY(int32 data,int16 *x_cm,int16 *y_cm)  //解析数据包函数
{
    // 右移16位得到X
    *x_cm = (int16)(data >> 16);

    // 直接取低16位得到Y
    *y_cm = (int16)data;
}

void Navigation_Update(void)
{
    // 保存上一次编码器值
    static int32last_encoder = 0;

    //--------------------------
    // 读取编码器
    //--------------------------

    int32now_encoder;

    now_encoder =
        Read_Encoder_Aggregate_Value(
            TIM5_ENCODER);

    // 本周期编码器增量
    int32delta_encoder;

    delta_encoder =
        now_encoder
        - last_encoder;

    last_encoder =
        now_encoder;

    //--------------------------
    // 计算行驶距离
    //--------------------------

    float ds;

    ds =
        delta_encoder
        * WHEEL_C
        / ENCODER_PPR;

    //--------------------------
    // 平均航向
    //--------------------------

    float delta_yaw;

    delta_yaw =angle_diff(Yaw,last_yaw);

    float yaw_avg;

    yaw_avg =
        last_yaw
        + delta_yaw / 2.0f;

    last_yaw = Yaw;

    //--------------------------
    // 转弧度
    //--------------------------

    float yaw_rad;

    yaw_rad =
        yaw_avg
        * PI
        / 180.0f;

    //--------------------------
    // 更新位置
    //--------------------------

    pos_x +=
        ds * sinf(yaw_rad);

    pos_y +=
        ds * cosf(yaw_rad);
}

************************************************************/

uint8 INS_Flag=0;

static void Enc_Tick(void);  // 前置声明(定义在文件末尾)

static uint16 stuck_counter = 0;  // 超时计数器(10ms中断累加)
#define STUCK_TIMEOUT      300   // 超时阈值: 300 x 10ms = 3秒
#define GPS_FORCE_DELAY    50    // GPS介入延迟: 50 x 10ms = 0.5秒

float gps_fusion_gain = 0.00f;          // 互补滤波GPS权重(可调)
float diff_gain = 0.35f;           // diff gain (0.0=off, default 0.35, max ~0.5)
int32 km3_fwd_speed = 500;   // KM3 forward speed
int32 km3_rev_speed  = 250;   // KM3 reverse speed (absolute)
uint8 gps_confirm_mode = 0;             // 0=纯惯导, 1=GPS辅助


/************************************************
 *  函数名称: 纯编码器和陀螺仪转二维坐标函数
 *  功能说明：
 *  参数说明：无
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年6月17日
 *  备   注：放中断执行，10ms
 ************************************************/
float INS_X = 0.0f;      // X坐标(m)
float INS_Y = 0.0f;      // Y坐标(m)
float INS_Yaw = 0.0f;    // 航向角(单位是rad)
int32 INS_Last_Encoder = 0;  // 上次编码器值
void INS_Update(void)
{
    // 读取累计编码器
    int32 Encoder_Now =(Read_Encoder_Aggregate_Value (TIM5_ENCODER) + Read_Encoder_Aggregate_Value(TIM2_ENCODER))/2;//左侧编码器+右侧编码器

    // 计算增量
    int32 delta_encoder = Encoder_Now - INS_Last_Encoder;

    INS_Last_Encoder = Encoder_Now;

    // 脉冲转距离
    float ds = ((WHEEL_C)/2048.0f)*delta_encoder;

    // 航向角转弧度
    float INS_Yaw =(Yaw * PI) / 180.0f;

    // 更新坐标
    INS_X += ds * sinf(INS_Yaw);

    INS_Y += ds * cosf(INS_Yaw);

    // ---- GPS组合导航校准（仅当gain>0时启用）----
    GPS_ToXY();                             // 更新 GPS_X, GPS_Y
    if(gnss.state == 1 && gps_origin_set && gps_fusion_gain > 0.0f)   // GPS有效且原点已设且开启GPS融合
    {
        // 互补滤波: INS往GPS方向缓慢拉近
        INS_X += (GPS_X - INS_X) * gps_fusion_gain;
        INS_Y += (GPS_Y - INS_Y) * gps_fusion_gain;

        // 车速>2km/h时用GPS地面航向校准Yaw漂移
        if(gnss.speed > 2.0f)
        {
            float yaw_err = angle_diff(gnss.direction, Yaw);
            if(fabs(yaw_err) < 30.0f)  // 过滤异常跳变
            {
                Yaw += yaw_err * 0.005f;
                if(Yaw >= 360.0f) Yaw -= 360.0f;
                if(Yaw < 0.0f)    Yaw += 360.0f;
            }
        }
    }

    // 超时计时(10ms中断): 循迹时累加, 用于防卡点
    if(INS_Flag == 2) stuck_counter++;

    // 编码器按键计时(10ms中断)
    Enc_Tick();
}


/************************************************
 *  函数名称: GPS经纬度转局部平面坐标
 *  功能说明：将当前GPS经纬度转为以设定原点为基准的二维坐标(m)
 *  参数说明：无
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年7月7日
 *  备   注：使用球面简化为平面近似，适用于短距离(几百米)
 ************************************************/
#define EARTH_R         6378137.0f      // 地球半径(m)
#define D2R             0.01745329252f  // 度转弧度

static float gps_origin_lat = 0.0f;     // GPS坐标原点纬度
static float gps_origin_lon = 0.0f;     // GPS坐标原点经度
float GPS_X = 0.0f;                     // GPS转换的东向坐标(m)
float GPS_Y = 0.0f;                     // GPS转换的北向坐标(m)
uint8 gps_origin_set = 0;               // 原点是否已设定

/************************************************
 *  函数名称: GPS_SetOrigin
 *  功能说明：设置GPS局部坐标系原点
 *  参数说明：无
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年7月7日
 *  备   注：发车时调用一次，将当前位置作为坐标原点
 ************************************************/
void GPS_SetOrigin(void)
{
    if(gnss.state == 1)
    {
        gps_origin_lat = (float)gnss.latitude;
        gps_origin_lon = (float)gnss.longitude;
        gps_origin_set = 1;
        GPS_X = 0.0f;
        GPS_Y = 0.0f;
    }
}

/************************************************
 *  函数名称: GPS_ToXY
 *  功能说明：将GPS当前经纬度转换为局部平面坐标(m)
 *  参数说明：无
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年7月7日
 *  备   注：结果写入全局变量 GPS_X, GPS_Y
 ************************************************/
void GPS_ToXY(void)
{
    if(gnss.state != 1 || !gps_origin_set) return;

    float dlat = (float)(gnss.latitude  - gps_origin_lat) * D2R;
    float dlon = (float)(gnss.longitude - gps_origin_lon) * D2R;
    float cos_lat0 = cosf(gps_origin_lat * D2R);

    GPS_X = EARTH_R * dlon * cos_lat0;   // 东向(m)
    GPS_Y = EARTH_R * dlat;              // 北向(m)
}


/************************************************
 *  函数名称: 计算俩个坐标之间的距离
 *  功能说明：
 *  参数说明： x1, y1：出发点坐标；x2, y2：目标点坐标；
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年6月17日
 *  备   注：单位（m）
 ************************************************/
float Get_Two_Point_Distance(float x1, float y1,float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;

    return sqrtf(dx * dx + dy * dy);
}


/************************************************
 *  函数名称: 计算俩个坐标之间的角度
 *  功能说明：
 *  参数说明：x1, y1：出发点坐标；x2, y2：目标点坐标；
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年6月17日
 *  备   注：范围0-360
 ************************************************/
float Get_Target_Angle(float x1, float y1,float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;

    // 注意参数顺序！
    float angle = atan2f(dx, dy) * 180.0f / PI;

    // 转成 0~360°
    if(angle < 0)
        angle += 360.0f;

    return angle;
}

/************************************************
 *  函数名称: 采集路线坐标
 *  功能说明：
 *  参数说明：无
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年6月17日
 *  备   注：
 ************************************************/
int32 INS_data[NagPoint_MAX] = {0};
int32 INS_Go_Point_Flag=0;
int32 INS_Back_Point_Flag=0;

// ==== GPS纯坐标路点（与INS并行存储，用于对照修正）====
int32 GPS_xy_data[NagPoint_MAX] = {0};     // GPS路点数组
int32 GPS_Go_Point_Flag=0;
int32 GPS_Back_Point_Flag=0;
int32 GPS_xy_data2[NagPoint_MAX] = {0};    // GPS读取缓冲
int32 GPS_Resd_GoPoint_Flag=0;
int32 GPS_Resd_BackPoint_Flag=0;

void INS_write(void)
{
    int32 delta2;
    int32 pack;

    Read_Continue_Encoder_Left=(Read_Encoder_Aggregate_Value(TIM5_ENCODER) + Read_Encoder_Aggregate_Value(TIM2_ENCODER))/2;//左侧编码器+右侧编码器
    delta2 = Read_Continue_Encoder_Left - Last_Encoder;

    if((abs(delta2) >= Nag_Set_mileage) && (INS_Go_Point_Flag + INS_Back_Point_Flag) < NagPoint_MAX)
    {


        int16 x_int = (int16)(INS_X * 100.0f);
        int16 y_int = (int16)(INS_Y * 100.0f);

        pack = (((uint32)(uint16)x_int) << 16) | ((uint16)y_int);

        INS_data2[INS_Go_Point_Flag + INS_Back_Point_Flag] = pack;

        // ---- 同步记录GPS纯坐标 ----
        int16 gx_int = (int16)(GPS_X * 100.0f);
        int16 gy_int = (int16)(GPS_Y * 100.0f);
        GPS_xy_data2[GPS_Go_Point_Flag + GPS_Back_Point_Flag] =
            (((uint32)(uint16)gx_int) << 16) | ((uint16)gy_int);

        if(delta2 >= 0)
        {
            INS_Go_Point_Flag++;
            GPS_Go_Point_Flag++;
        }
        else
        {
            INS_Back_Point_Flag++;
            GPS_Back_Point_Flag++;
        }
        Last_Encoder = Read_Continue_Encoder_Left;
        Buzzer_check(50);
    }
}

/************************************************
 *  函数名称: 存flash
 *  功能说明：
 *  参数说明：无
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年6月17日
 *  备   注：
 ************************************************/
void INS_FlashWrite(void)
{
    flash_buffer_clear();

    flash_union_buffer[500].uint32_type = INS_Go_Point_Flag;
    flash_union_buffer[501].uint32_type = INS_Back_Point_Flag;
    for(int i = 0;i<(INS_Go_Point_Flag + INS_Back_Point_Flag);i++)
    {
        flash_union_buffer[i].uint32_type =  INS_data2[i];
    }

    if(flash_check(0, 9))
    {
       flash_erase_page(0, 9);
    }

    flash_write_page_from_buffer(0, 9);

    ips200_show_string(0, 16*10, "INS flash");
}

/************************************************
 *  函数名称: 读取flash
 *  功能说明：
 *  参数说明：无
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年6月17日
 *  备   注：
 ************************************************/
int32 INS_data2[NagPoint_MAX] = {0};
int32 INS_Resd_GoPoint_Flag=0;
int32 INS_Resd_BackPoint_Flag=0;
void INS_FlashRead(void)
{
    if(flash_check(0, 9))
    {
        flash_read_page_to_buffer(0, 9);

        INS_Resd_GoPoint_Flag  = flash_union_buffer[500].uint32_type;
        INS_Resd_BackPoint_Flag = flash_union_buffer[501].uint32_type;

        /* 校验: 擦除页=0xFFFFFFFF=-1, 或超出范围则视为无效 */
        if(INS_Resd_GoPoint_Flag < 0 || INS_Resd_GoPoint_Flag > NagPoint_MAX)
            INS_Resd_GoPoint_Flag = 0;
        if(INS_Resd_BackPoint_Flag < 0 || INS_Resd_BackPoint_Flag > NagPoint_MAX)
            INS_Resd_BackPoint_Flag = 0;

        int32 total = INS_Resd_GoPoint_Flag + INS_Resd_BackPoint_Flag;
        for(int i = 0; i < total; i++)
        {
            INS_data2[i] = flash_union_buffer[i].uint32_type;
        }

        /* 同步录点计数, 确保保存时使用正确的 count */
        INS_Go_Point_Flag  = INS_Resd_GoPoint_Flag;
        INS_Back_Point_Flag = INS_Resd_BackPoint_Flag;

        ips200_show_string(172, 16*11, total > 0 ? "OK flash" : "NO data");
    }
    else
    {
        ips200_show_string(172, 16*11, "NO flash");
    }
}

/************************************************
 *  函数名称: GPS纯坐标存flash (Page 25, 与INS不同扇区)
 *  功能说明：存储GPS二维坐标路点
 *  参数说明：无
 *  修改时间：2026年7月9日
 *  备   注：Page 25 与 Page 9(INS) 在不同扇区, 避免互相擦除
 ************************************************/
void GPS_FlashWrite(void)
{
    flash_buffer_clear();

    flash_union_buffer[500].uint32_type = GPS_Go_Point_Flag;
    flash_union_buffer[501].uint32_type = GPS_Back_Point_Flag;
    for(int i = 0; i < (GPS_Go_Point_Flag + GPS_Back_Point_Flag); i++)
    {
        flash_union_buffer[i].uint32_type = GPS_xy_data2[i];
    }

    if(flash_check(0, 25))
    {
       flash_erase_page(0, 25);
    }

    flash_write_page_from_buffer(0, 25);

    ips200_show_string(0, 16*10, "GPS flash");
}

/************************************************
 *  函数名称: GPS纯坐标读flash (Page 25, 与INS不同扇区)
 *  功能说明：读取GPS二维坐标路点
 *  参数说明：无
 *  修改时间：2026年7月9日
 *  备   注：
 ************************************************/
void GPS_FlashRead(void)
{
    if(flash_check(0, 25))
    {
        flash_read_page_to_buffer(0, 25);

        GPS_Resd_GoPoint_Flag  = flash_union_buffer[500].uint32_type;
        GPS_Resd_BackPoint_Flag = flash_union_buffer[501].uint32_type;

        /* 校验: 擦除页=0xFFFFFFFF=-1, 或超出范围则视为无效 */
        if(GPS_Resd_GoPoint_Flag < 0 || GPS_Resd_GoPoint_Flag > NagPoint_MAX)
            GPS_Resd_GoPoint_Flag = 0;
        if(GPS_Resd_BackPoint_Flag < 0 || GPS_Resd_BackPoint_Flag > NagPoint_MAX)
            GPS_Resd_BackPoint_Flag = 0;

        int32 total = GPS_Resd_GoPoint_Flag + GPS_Resd_BackPoint_Flag;
        for(int i = 0; i < total; i++)
        {
            GPS_xy_data2[i] = flash_union_buffer[i].uint32_type;
        }

        /* 同步录点计数, 确保保存时使用正确的 count */
        GPS_Go_Point_Flag  = GPS_Resd_GoPoint_Flag;
        GPS_Back_Point_Flag = GPS_Resd_BackPoint_Flag;

        ips200_show_string(172, 16*11, total > 0 ? "OK GPS" : "NO data");
    }
    else
    {
        ips200_show_string(172, 16*11, "NO GPS");
    }
}

/************************************************
 *  函数名称: GPS整体偏移修正INS路径
 *  功能说明：用GPS路点的平均偏移量整体平移INS路点
 *             INS管形状，GPS管定位，各取所长
 *  参数说明：无
 *  修改时间：2026年7月7日
 *  备   注：操作一次，全点平移，100个点和10个点一样
 ************************************************/
void GPS_Correct_INS(void)
{
    int32 total = GPS_Resd_GoPoint_Flag + GPS_Resd_BackPoint_Flag;
    if(total == 0) return;

    // 累加所有点的GPS-INS偏差
    float sum_dx = 0, sum_dy = 0;
    for(int i = 0; i < total; i++)
    {
        int16 ix = (int16)(INS_data2[i] >> 16);
        int16 iy = (int16)(INS_data2[i] & 0xFFFF);
        int16 gx = (int16)(GPS_xy_data2[i] >> 16);
        int16 gy = (int16)(GPS_xy_data2[i] & 0xFFFF);

        sum_dx += (float)(gx - ix);
        sum_dy += (float)(gy - iy);
    }

    // 平均偏移量(cm)
    int16 dx = (int16)(sum_dx / total);
    int16 dy = (int16)(sum_dy / total);

    // 全部INS路点整体平移
    for(int i = 0; i < total; i++)
    {
        int16 x = (int16)(INS_data2[i] >> 16) + dx;
        int16 y = (int16)(INS_data2[i] & 0xFFFF) + dy;
        INS_data2[i] = (((uint32)(uint16)x) << 16) | ((uint16)y);
    }

    // 显示修正信息: 方向 + 数值
    ips200_show_string(230, 16*8, "C");
    // X方向: R=GPS在INS右边, L=GPS在INS左边
    ips200_show_string(0,   16*9,  (dx >= 0) ? "GPS-R:" : "GPS-L:");
    ips200_show_float( 60,  16*9,  fabs(dx) / 100.0f, 2, 2);  // 显示为米
    // Y方向: F=GPS在INS前方, B=GPS在INS后方
    ips200_show_string(120,  16*9, (dy >= 0) ? "GPS-F:" : "GPS-B:");
    ips200_show_float( 180, 16*9,  fabs(dy) / 100.0f, 2, 2);
    // 总偏移距离
    float total_shift = sqrtf((float)(dx*dx + dy*dy)) / 100.0f;
    ips200_show_string(0,   16*10, "Shift:");
    ips200_show_float( 60,  16*10, total_shift, 2, 2);
    ips200_show_string(120,  16*10, "m");
}

/************************************************
 *  函数名称: INS修正GPS路径（反向）
 *  功能说明：GPS整体偏移时，用INS路点修正GPS路点
 *            适用于GPS原点设偏、卫星几何差等场景
 *  参数说明：无
 *  修改时间：2026年7月7日
 *  备   注：与GPS_Correct_INS方向相反
 ************************************************/
void INS_Correct_GPS(void)
{
    int32 total = GPS_Resd_GoPoint_Flag + GPS_Resd_BackPoint_Flag;
    if(total == 0) return;

    float sum_dx = 0, sum_dy = 0;
    for(int i = 0; i < total; i++)
    {
        int16 ix = (int16)(INS_data2[i] >> 16);
        int16 iy = (int16)(INS_data2[i] & 0xFFFF);
        int16 gx = (int16)(GPS_xy_data2[i] >> 16);
        int16 gy = (int16)(GPS_xy_data2[i] & 0xFFFF);
        sum_dx += (float)(ix - gx);   // 注意：方向反了
        sum_dy += (float)(iy - gy);
    }

    int16 dx = (int16)(sum_dx / total);
    int16 dy = (int16)(sum_dy / total);

    // GPS路点整体平移
    for(int i = 0; i < total; i++)
    {
        int16 x = (int16)(GPS_xy_data2[i] >> 16) + dx;
        int16 y = (int16)(GPS_xy_data2[i] & 0xFFFF) + dy;
        GPS_xy_data2[i] = (((uint32)(uint16)x) << 16) | ((uint16)y);
    }

    ips200_show_string(230, 16*8, "c");
    ips200_show_string(0,   16*9,  (dx >= 0) ? "INS-R:" : "INS-L:");
    ips200_show_float( 60,  16*9,  fabs(dx) / 100.0f, 2, 2);
    ips200_show_string(120,  16*9, (dy >= 0) ? "INS-F:" : "INS-B:");
    ips200_show_float( 180, 16*9,  fabs(dy) / 100.0f, 2, 2);
}

/************************************************
 *  函数名称: GPS直接替换INS路径
 *  功能说明：INS路径完全不可靠时，直接用GPS路点替换
 *  参数说明：无
 *  修改时间：2026年7月7日
 *  备   注：GPS噪声1~3m，方向盘会有些抖，作为兜底方案
 ************************************************/
void GPS_Replace_INS(void)
{
    int32 total = GPS_Resd_GoPoint_Flag + GPS_Resd_BackPoint_Flag;
    if(total == 0) return;

    for(int i = 0; i < total; i++)
    {
        INS_data2[i] = GPS_xy_data2[i];
    }
    INS_Resd_GoPoint_Flag  = GPS_Resd_GoPoint_Flag;
    INS_Resd_BackPoint_Flag = GPS_Resd_BackPoint_Flag;

    ips200_show_string(230, 16*8, "R");
}

void reverse_path_xy(int32 *path, int32 len)  //二维坐标反转函数
{
    if(len == 0) return;

    int16 end_x = (int16)(path[len - 1] >> 16);
    int16 end_y = (int16)(path[len - 1] & 0xFFFF);

    // 倒序
    for(uint16 i = 0; i < len / 2; i++)
    {
        uint32 temp = path[i];
        path[i] = path[len - 1 - i];
        path[len - 1 - i] = temp;
    }

    // 去掉新的起点并坐标变换
    for(uint16 i = 0; i < len - 1; i++)
    {
        int16 x = (int16)(path[i + 1] >> 16);
        int16 y = (int16)(path[i + 1] & 0xFFFF);

        x -= end_x;
        y -= end_y;

        path[i] = (((uint32)(uint16)x) << 16) |
                   ((uint16)y);
    }

    // 最后一个目标点：原来的(0,0)
    path[len - 1] =
        (((uint32)(uint16)(-end_x)) << 16) |
         ((uint16)(-end_y));
}

float INS_two_points_distance = 0;

// 超时强制切点: stuck_counter在10ms中断累加, 这里只读
static int16  stuck_waypoint = -1;

void INS_track(void)
{

    // 边界保护: next越界则直接停车(放最前面防止越界读数组)
    int32 total_pts = INS_Resd_GoPoint_Flag + INS_Resd_BackPoint_Flag;
    if(next >= total_pts)
    {
        dispatch_car = 0;
        Back_Motor_SetPwm(0,0);
        Angle_Target_Position = 0;
        return;
    }

    int32 pack = INS_data2[next];
    int16 x_int = (int16)(pack >> 16);
    int16 y_int = (int16)(pack & 0xFFFF);
    float x = x_int / 100.0f;
    float y = y_int / 100.0f;
    float INS_Error=0;
    float INS_azimuth = 0;

    INS_two_points_distance = Get_Two_Point_Distance(INS_X,INS_Y,x,y); //获取俩点间距离
    INS_azimuth = Get_Target_Angle(INS_X,INS_Y,x,y);    //获取俩点之间的角度

    // 防打滑: GPS双重校验
    // gps_confirm: 宽松校验(3m), 防止打滑时INS误切点
    // gps_force:   强力校验(1.5m), 倒车打滑时GPS直接切点
    float gps_to_target = Get_Two_Point_Distance(GPS_X, GPS_Y, x, y);
    // gps_confirm: 可切换模式
    uint8 gps_confirm = (gps_confirm_mode == 0) ? 1 :
                        ((gnss.state == 1) ? (gps_to_target < 2.2f) : 1);
    // 强制切点: 前进段严格, 倒车段宽松(GPS噪声大)
    uint8 is_forward  = (next < INS_Resd_GoPoint_Flag) ? 1 : 0;
    // 强制切点已关闭, 只用正常路径
    uint8 gps_force   = 0;
    uint8 stuck_force = 0;

    ips200_show_string(0, 16*3, "x:");
    ips200_show_string(0, 16*4, "Y:");
    ips200_show_float( 60, 16*3,x, 3,1);
    ips200_show_float( 60, 16*4,y, 3,1);

    ips200_show_string(0, 16*5, "INS_X:");
    ips200_show_string(0, 16*6, "INS_Y:");
    ips200_show_float( 60, 16*5,INS_X, 3,1);
    ips200_show_float( 60, 16*6,INS_Y, 3,1);

    // GPS校验状态显示
    ips200_show_string(0, 16*7, "GPS D:");
    ips200_show_float(60, 16*7, gps_to_target, 3, 1);
    if(gps_confirm)
        ips200_show_string(120, 16*7, "OK");     // 正常
    else
        ips200_show_string(120, 16*7, "WAIT");   // 打滑中，等GPS确认

    if((INS_two_points_distance < INS_distance  && next==0 && gps_confirm) || gps_force || stuck_force)
    {
       next++;
       stuck_force = 0;
       gps_force = 0;
       stuck_counter = 0;

       Back_Motor_PID_Start_flag = 1; //为1时开启位置PID
       Back_Motor_Set_Speed = km3_fwd_speed;//set speed
       Buzzer_SetTimer(50);
    }
    if((INS_two_points_distance < INS_distance &&  next!=0 &&  next < INS_Resd_GoPoint_Flag && gps_confirm) || (gps_force && is_forward) || (stuck_force && is_forward))
    {
       next++;
       stuck_force = 0;
       gps_force = 0;
       stuck_counter = 0;
       Back_Motor_Set_Speed = km3_fwd_speed;//set speed
       Buzzer_SetTimer(50);
    }

    // 倒车阈值: GPS确认时放宽到1.5m, 否则0.8m
    {
       float rev_threshold = gps_confirm ? 1.5f : 1.5f;
       if((INS_two_points_distance < rev_threshold &&  next!=0 &&  (next > INS_Resd_GoPoint_Flag && next<INS_Resd_GoPoint_Flag + INS_Resd_BackPoint_Flag-1) && gps_confirm) || (gps_force && !is_forward) || (stuck_force && !is_forward))
       {
          next++;
          stuck_force = 0;
          gps_force = 0;
          stuck_counter = 0;
          Back_Motor_Set_Speed = -km3_rev_speed;//set speed
          Buzzer_SetTimer(50);
       }
    }

      //电机速度模式选择 + 方向PID
    if(dispatch_car == 1)
    {
        // 先计算航向误差（电机差速和舵机共用）
        if(INS_Resd_BackPoint_Flag !=0 && (next > INS_Resd_GoPoint_Flag && next<INS_Resd_GoPoint_Flag + INS_Resd_BackPoint_Flag-1))
        {
            INS_Error = -angle_diff(INS_azimuth,((Yaw+180) >=360 ) ? (Yaw-180) : (Yaw+180));
        }
        else
        {
            INS_Error = angle_diff(INS_azimuth,Yaw);    // 方向角与航向角之间的误差（用于控制）
        }

        Angle_Target_Position = -GPS_IMU_PID_Start(Open,INS_Error,0,1);
    }

    if(next >= 1 && next != INS_Resd_GoPoint_Flag + INS_Resd_BackPoint_Flag) //flag_speed如果是0，开启PID
    {
        // 倒车入库阶段不需要差速，用等速（覆盖全部倒车路点）
        if(INS_Resd_BackPoint_Flag !=0 && (next > INS_Resd_GoPoint_Flag))
        {
            Back_Motor_SetPwm(Back_Motor_Set_Speed, Back_Motor_Set_Speed);
        }
        else
        {
            Back_Motor_SetPwm_Diff(Back_Motor_Set_Speed, INS_Error);      // 差速转向
        }
    }

    // 前进→倒车过渡: 不用强制切，只用正常路径(唯一一次方向切换，必须精准)
    if(INS_two_points_distance < 0.8  && next == INS_Resd_GoPoint_Flag && INS_Resd_BackPoint_Flag !=0 && gps_confirm)
    {
        next++;
        Back_Motor_SetPwm(0,0);
        Buzzer_SetTimer(50);
        system_delay_ms(400);
        stuck_counter = 0;          // delay后再清零, 避免ISR累加干扰

        Back_Motor_Set_Speed = -km3_rev_speed;//set speed
    }
    // 最后停车: 必须精准(不用强制切)
    if(INS_two_points_distance < INS_distance_final  && next >= INS_Resd_GoPoint_Flag + INS_Resd_BackPoint_Flag-1 && gps_confirm)
    {
        next = INS_Resd_GoPoint_Flag + INS_Resd_BackPoint_Flag;
        stuck_force = 0;
        gps_force = 0;
        stuck_counter = 0;
        delay_ScanTemp = 0;//关闭定时器延迟发车标志位
        dispatch_car = 0;//关闭发车标志位
        Back_Motor_Set_Speed = 0;//设置速度
        Back_Motor_SetPwm(0,0);
        Angle_Target_Position = 0;  // 方向盘回正
        Back_Motor_PID_Start_flag = 0; //关闭位置PID，给电机速度为0
        INS_Flag = 0;
        Buzzer_SetTimer(1000);
    }

}

/************************************************
 *  函数名称: 差速转向PWM设置
 *  功能说明：根据角度误差分配左右轮速度差，减小转弯半径
 *  参数说明：base_speed=基础速度, angle_error=当前航向与目标航向的角度差(-180~180)
 *  修改时间：2026年7月10日
 *  备   注：误差越大差速越大，正误差=右转时左轮快右轮慢
 ************************************************/
void Back_Motor_SetPwm_Diff(int32 base_speed, float angle_error)
{
    // 将误差归一化到 -1.0 ~ 1.0（90度封顶）
    float ratio = angle_error / 60.0f;
    if(ratio > 1.0f)  ratio = 1.0f;
    if(ratio < -1.0f) ratio = -1.0f;

    // 差速量 = 基础速度绝对值 × 增益 × 误差比例
    int32 diff = (int32)((float)abs(base_speed) * diff_gain * fabs(ratio));

    int32 left, right;
    if(angle_error > 0)  // 需要左转：右轮快、左轮慢
    {
        left  = base_speed - diff;
        right = base_speed + diff;
    }
    else                 // 需要右转：左轮快、右轮慢
    {
        left  = base_speed + diff;
        right = base_speed - diff;
    }

    Back_Motor_SetPwm(left, right);
}


/************************************************
 *  函数名称: 二维坐标惯导执行函数
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年6月17日
 *  备   注：
 ************************************************/
void INS_choise(void)
{
    switch(INS_Flag)
   {
        case 0:
            break;

        case 1:
            INS_write();
          break;

        case 2:
            INS_track();
            break;

        case 3:
            Waypoint_Editor();
            break;

   }
}


/************************************************
 *  函数名称: 二维坐标惯导按键
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年5月26日
 *  备   注：
 ************************************************/
void System_INS(void)
{
    if(!gpio_get_level(sw1) &&  !gpio_get_level(sw2))//1,2上
    {
        ips200_show_string(230, 16*8, "S");
        if(key1_flag == 1 && INS_Flag == 0)
        {
            key1_flag=0;
            INS_Flag=1;
            next = 0;               // 复位路点索引
            // 复位所有采点计数器
            INS_Go_Point_Flag = 0;
            INS_Back_Point_Flag = 0;
            GPS_Go_Point_Flag = 0;
            GPS_Back_Point_Flag = 0;
            GPS_SetOrigin();        // 设定GPS坐标原点
            INS_X = 0; INS_Y = 0;   // 同时清零INS坐标

            INS_Last_Encoder = (Read_Encoder_Aggregate_Value(TIM5_ENCODER)+Read_Encoder_Aggregate_Value(TIM2_ENCODER))/2; // 同步编码器基准
            Buzzer_check(50);
        }
        if(key2_flag == 1)
        {
            key2_flag=0;
            INS_FlashWrite();   // INS坐标存flash(Page 9)
            GPS_FlashWrite();   // GPS坐标存flash(Page 10)
            INS_Flag=0;
            Buzzer_check(50);
        }
        if(key3_flag == 1)
        {
            key3_flag=0;
            INS_FlashRead();    // INS坐标读flash(Page 9)
            GPS_FlashRead();    // GPS坐标读flash(Page 10)
            next = 0;           // 复位路点索引
            INS_Flag=0;
            Buzzer_check(50);
        }
        if(key4_flag == 1)
        {
            key4_flag=0;
            INS_Flag=2;
            next = 0;               // 复位路点索引
            stuck_waypoint = -1;    // 复位超时检测
            stuck_counter = 0;
            dispatch_car= 1;
            GPS_SetOrigin();        // 设定GPS坐标原点
            Back_Motor_Set_Speed=200;
            Back_Motor_SetPwm(200,200);
            Buzzer_check(50);
        }
    }
    else if(gpio_get_level(sw1) &&  gpio_get_level(sw2))//1,2下
    {
        ips200_show_string(230, 16*8, "X"); //原点清零
        if(key1_flag == 1)
        {
            key1_flag=0;
            INS_X = 0;
            INS_Y = 0;
            GPS_SetOrigin();        // 重设GPS原点
            Buzzer_check(50);
        }
        if(key2_flag == 1)      //反转坐标函数
        {
            ips200_show_string(230, 16*9, "F"); //原点清零
            Buzzer_check(50);
            key2_flag=0;
            reverse_path_xy(INS_data2, INS_Resd_GoPoint_Flag + INS_Resd_BackPoint_Flag);
        }
        if(key3_flag == 1)          // GPS整体偏移修正INS
        {
            Buzzer_check(50);
            key3_flag=0;
            GPS_Correct_INS();
        }
        if(key4_flag == 1)          // INS修正GPS(反向)
        {
            Buzzer_check(50);
            key4_flag=0;
            INS_Correct_GPS();
        }

    }
    else if(gpio_get_level(sw1) &&  !gpio_get_level(sw2))//1下,2上
    {
        ips200_show_string(230, 16*8, "X");  // reverse修正

        if(key1_flag == 1)          // 画路径对比图
        {
            Buzzer_check(50);
            key1_flag=0;
            INS_GPS_Display();
        }
        if(key2_flag == 1)          // 进入编辑器模式
        {
            Buzzer_check(50);
            key2_flag=0;
            INS_Flag = 3;           // 编辑器持续运行
        }
    }
    else if(!gpio_get_level(sw1) &&  gpio_get_level(sw2))//1shang,2下
    {
        if(key2_flag == 1)          // GPS替换INS
        {
            Buzzer_check(50);
            key2_flag=0;
            GPS_Replace_INS();
        }
    }
}

/************************************************
 *  函数名称: INS与GPS路径对比显示
 *  功能说明：屏幕画出INS(蓝)和GPS(红)两套路点
 *            点数不固定，自动缩放适配
 *  修改时间：2026年7月7日
 *  备   注：类似KM2画图，直接画XY坐标
 ************************************************/
void INS_GPS_Display(void)
{
    int32 total = INS_Resd_GoPoint_Flag + INS_Resd_BackPoint_Flag;
    if(total < 2) return;
    if(total > NagPoint_MAX) total = NagPoint_MAX;

    // 绘图有效区(留边距3px防断言)，IPS200: 240x320
    #define DRAW_X_MIN  3
    #define DRAW_X_MAX  236
    #define DRAW_Y_MIN  16
    #define DRAW_Y_MAX  315
    #define DRAW_W      (DRAW_X_MAX - DRAW_X_MIN)
    #define DRAW_H      (DRAW_Y_MAX - DRAW_Y_MIN)

    // 第1遍: 算XY范围
    int16 min_x = 32767, max_x = -32768;
    int16 min_y = 32767, max_y = -32768;
    int16 ix, iy, gx, gy;

    for(int i = 0; i < total; i++)
    {
        ix = (int16)(INS_data2[i] >> 16);
        iy = (int16)(INS_data2[i] & 0xFFFF);
        gx = (int16)(GPS_xy_data2[i] >> 16);
        gy = (int16)(GPS_xy_data2[i] & 0xFFFF);

        if(ix < min_x) min_x = ix; if(ix > max_x) max_x = ix;
        if(iy < min_y) min_y = iy; if(iy > max_y) max_y = iy;
        if(gx < min_x) min_x = gx; if(gx > max_x) max_x = gx;
        if(gy < min_y) min_y = gy; if(gy > max_y) max_y = gy;
    }

    int16 range_x = max_x - min_x; if(range_x == 0) range_x = 1;
    int16 range_y = max_y - min_y; if(range_y == 0) range_y = 1;
    float scale_x = (float)DRAW_W / range_x;
    float scale_y = (float)DRAW_H / range_y;
    float scale = (scale_x < scale_y) ? scale_x : scale_y;

    int16 off_x = (int16)(((float)DRAW_W - range_x * scale) / 2.0f);
    int16 off_y = (int16)(((float)DRAW_H - range_y * scale) / 2.0f);

    int16 sx0, sy0, sx1, sy1;

    // 画GPS路径(红色)
    for(int i = 0; i < total - 1; i++)
    {
        sx0 = (int16)(DRAW_X_MIN + off_x + ((int16)(GPS_xy_data2[i]   >> 16) - min_x) * scale);
        sy0 = (int16)(DRAW_Y_MIN + DRAW_H - off_y - ((int16)(GPS_xy_data2[i]   & 0xFFFF) - min_y) * scale);
        sx1 = (int16)(DRAW_X_MIN + off_x + ((int16)(GPS_xy_data2[i+1] >> 16) - min_x) * scale);
        sy1 = (int16)(DRAW_Y_MIN + DRAW_H - off_y - ((int16)(GPS_xy_data2[i+1] & 0xFFFF) - min_y) * scale);
        if(sx0 < DRAW_X_MIN) sx0 = DRAW_X_MIN; if(sx0 > DRAW_X_MAX) sx0 = DRAW_X_MAX;
        if(sy0 < DRAW_Y_MIN) sy0 = DRAW_Y_MIN; if(sy0 > DRAW_Y_MAX) sy0 = DRAW_Y_MAX;
        if(sx1 < DRAW_X_MIN) sx1 = DRAW_X_MIN; if(sx1 > DRAW_X_MAX) sx1 = DRAW_X_MAX;
        if(sy1 < DRAW_Y_MIN) sy1 = DRAW_Y_MIN; if(sy1 > DRAW_Y_MAX) sy1 = DRAW_Y_MAX;

        ips200_draw_line((uint16)sx0, (uint16)sy0, (uint16)sx1, (uint16)sy1, RGB565_RED);
    }

    // 画INS路径(蓝色)
    for(int i = 0; i < total - 1; i++)
    {
        sx0 = (int16)(DRAW_X_MIN + off_x + ((int16)(INS_data2[i]   >> 16) - min_x) * scale);
        sy0 = (int16)(DRAW_Y_MIN + DRAW_H - off_y - ((int16)(INS_data2[i]   & 0xFFFF) - min_y) * scale);
        sx1 = (int16)(DRAW_X_MIN + off_x + ((int16)(INS_data2[i+1] >> 16) - min_x) * scale);
        sy1 = (int16)(DRAW_Y_MIN + DRAW_H - off_y - ((int16)(INS_data2[i+1] & 0xFFFF) - min_y) * scale);
        if(sx0 < DRAW_X_MIN) sx0 = DRAW_X_MIN; if(sx0 > DRAW_X_MAX) sx0 = DRAW_X_MAX;
        if(sy0 < DRAW_Y_MIN) sy0 = DRAW_Y_MIN; if(sy0 > DRAW_Y_MAX) sy0 = DRAW_Y_MAX;
        if(sx1 < DRAW_X_MIN) sx1 = DRAW_X_MIN; if(sx1 > DRAW_X_MAX) sx1 = DRAW_X_MAX;
        if(sy1 < DRAW_Y_MIN) sy1 = DRAW_Y_MIN; if(sy1 > DRAW_Y_MAX) sy1 = DRAW_Y_MAX;

        ips200_draw_line((uint16)sx0, (uint16)sy0, (uint16)sx1, (uint16)sy1, RGB565_BLUE);
    }

    // 起点(绿) 终点(黄)
    sx0 = (int16)(DRAW_X_MIN + off_x + ((int16)(INS_data2[0] >> 16) - min_x) * scale);
    sy0 = (int16)(DRAW_Y_MIN + DRAW_H - off_y - ((int16)(INS_data2[0] & 0xFFFF) - min_y) * scale);
    sx1 = (int16)(DRAW_X_MIN + off_x + ((int16)(INS_data2[total-1] >> 16) - min_x) * scale);
    sy1 = (int16)(DRAW_Y_MIN + DRAW_H - off_y - ((int16)(INS_data2[total-1] & 0xFFFF) - min_y) * scale);
    if(sx0 < DRAW_X_MIN) sx0 = DRAW_X_MIN; if(sx0 > DRAW_X_MAX) sx0 = DRAW_X_MAX;
    if(sy0 < DRAW_Y_MIN) sy0 = DRAW_Y_MIN; if(sy0 > DRAW_Y_MAX) sy0 = DRAW_Y_MAX;
    if(sx1 < DRAW_X_MIN) sx1 = DRAW_X_MIN; if(sx1 > DRAW_X_MAX) sx1 = DRAW_X_MAX;
    if(sy1 < DRAW_Y_MIN) sy1 = DRAW_Y_MIN; if(sy1 > DRAW_Y_MAX) sy1 = DRAW_Y_MAX;

    if(sx0 > DRAW_X_MIN+4 && sx0 < DRAW_X_MAX-4 && sy0 > DRAW_Y_MIN+4 && sy0 < DRAW_Y_MAX-4)
        ips200_draw_circle((uint16)sx0, (uint16)sy0, 4, RGB565_GREEN);
    if(sx1 > DRAW_X_MIN+4 && sx1 < DRAW_X_MAX-4 && sy1 > DRAW_Y_MIN+4 && sy1 < DRAW_Y_MAX-4)
        ips200_draw_circle((uint16)sx1, (uint16)sy1, 4, RGB565_YELLOW);

    // 图例(顶部)
    ips200_show_string(0, 0, "P:");
    ips200_show_int(20, 0, total, 4);
    ips200_show_string(70, 0, "Blue:INS Red:GPS");
}

// ============ 旋转编码器路点编辑器 ============

// 按键计时器(在INS_Update 10ms中断中累加)
static uint16 enc_btn_press_time = 0;

/************************************************
 *  函数名称: 编码器初始化
 ************************************************/
static void Enc_GPIO_Init(void)
{
    gpio_init(P13_2,  GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(P21_6,  GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(P11_11, GPI, GPIO_HIGH, GPI_PULL_UP);
}

/************************************************
 *  函数名称: 旋转编码器读数(灰度码)
 ************************************************/
static int8 Encoder_Read(void)
{
    static const int8 enc_states[16] = {0,1,-1,0,-1,0,0,1, 1,0,0,-1,0,-1,1,0};
    static uint8 last_state = 0;
    uint8 a = gpio_get_level(P13_2);
    uint8 b = gpio_get_level(P21_6);
    uint8 state = (a << 1) | b;
    int8 result = enc_states[(last_state << 2) | state];
    last_state = state;
    return result;
}

/************************************************
 *  函数名称: 编码器按键读取
 ************************************************/
static uint8 Enc_Button_Short(void)
{
    static uint8 last = 1;
    uint8 now = gpio_get_level(P11_11);
    uint8 ret = 0;

    if(last == 1 && now == 0)
        enc_btn_press_time = 0;
    else if(last == 0 && now == 1)
    {
        if(enc_btn_press_time < 100)
            ret = 1;
    }
    last = now;
    return ret;
}

static int8 enc_accumulator = 0;  // 编码器累积(ISR中更新, 主循环读取清零)

/* 路点标记数组 (KM3和KM2编辑器共用) */
static uint8 waypoint_mark[NagPoint_MAX] = {0};
static uint8 mark_count = 0;

/************************************************
 *  函数名称: 编码器按键Tick(10ms中断调用)
 ************************************************/
static void Enc_Tick(void)
{
    if(gpio_get_level(P11_11) == 0)
        enc_btn_press_time++;

    // 编码器读数也在中断中做(100Hz, 不会漏脉冲)
    enc_accumulator += Encoder_Read();
}

/************************************************
 *  函数名称: 路点编辑器
 ************************************************/
void Waypoint_Editor(void)
{
    static int16 edit_idx = 0;
    static uint8 edit_mode = 0;
    static uint8 edit_ins = 1;
    static int8 enc_acc = 0;
    static uint8 first_run = 1;

    // key1退出编辑器
    if(key1_flag == 1)
    {
        key1_flag = 0;
        INS_Flag = 0;
        first_run = 1;
        Buzzer_check(50);
        return;
    }

    int32 total = INS_Resd_GoPoint_Flag + INS_Resd_BackPoint_Flag;
    if(total == 0) return;

    if(first_run)
    {
        Enc_GPIO_Init();
        edit_idx = 0;
        edit_mode = 0;
        enc_acc = 0;
        enc_btn_press_time = 0;
        first_run = 0;
        ips200_full(RGB565_BLACK);
    }

    // 拨码切换编辑数据集
    if(!gpio_get_level(sw1) && gpio_get_level(sw2))
        edit_ins = 1;
    else if(gpio_get_level(sw1) && !gpio_get_level(sw2))
        edit_ins = 0;

    // 编码器累积(在10ms中断中更新, 这里读出并清零)
    enc_acc += enc_accumulator;
    enc_accumulator = 0;

    // 按钮
    uint8 btn = Enc_Button_Short();

    // 状态机
    if(edit_mode == 0)  // BROWSE
    {
        if(abs(enc_acc) >= 2)
        {
            int8 dir = (enc_acc > 0) ? 1 : -1;
            edit_idx += dir;
            if(edit_idx < 0) edit_idx = total - 1;
            if(edit_idx >= total) edit_idx = 0;
            enc_acc = 0;
        }
        if(btn == 1) { edit_mode = 1; enc_acc = 0; }
    }
    else if(edit_mode == 1)  // EDIT_X
    {
        if(abs(enc_acc) >= 2)
        {
            float step = WP_EDIT_STEP * 100.0f;
            int8 dir = (enc_acc > 0) ? 1 : -1;
            if(edit_ins)
            {
                int16 x = (int16)(INS_data2[edit_idx] >> 16) + (int16)(dir * step);
                int16 y = (int16)(INS_data2[edit_idx] & 0xFFFF);
                INS_data2[edit_idx] = (((uint32)(uint16)x) << 16) | ((uint16)y);
            }
            else
            {
                int16 x = (int16)(GPS_xy_data2[edit_idx] >> 16) + (int16)(dir * step);
                int16 y = (int16)(GPS_xy_data2[edit_idx] & 0xFFFF);
                GPS_xy_data2[edit_idx] = (((uint32)(uint16)x) << 16) | ((uint16)y);
            }
            enc_acc = 0;
        }
        if(btn == 1) { edit_mode = 2; enc_acc = 0; }
    }
    else if(edit_mode == 2)  // EDIT_Y
    {
        if(abs(enc_acc) >= 2)
        {
            float step = WP_EDIT_STEP * 100.0f;
            int8 dir = (enc_acc > 0) ? 1 : -1;
            if(edit_ins)
            {
                int16 x = (int16)(INS_data2[edit_idx] >> 16);
                int16 y = (int16)(INS_data2[edit_idx] & 0xFFFF) + (int16)(dir * step);
                INS_data2[edit_idx] = (((uint32)(uint16)x) << 16) | ((uint16)y);
            }
            else
            {
                int16 x = (int16)(GPS_xy_data2[edit_idx] >> 16);
                int16 y = (int16)(GPS_xy_data2[edit_idx] & 0xFFFF) + (int16)(dir * step);
                GPS_xy_data2[edit_idx] = (((uint32)(uint16)x) << 16) | ((uint16)y);
            }
            enc_acc = 0;
        }
        if(btn == 1) { edit_mode = 0; enc_acc = 0; }
    }

    // ---- 画路径图(上半屏 0~180) ----
    {
        static int16 last_idx = -1, last_mode = -1;
        static uint8 last_ins = 255;
        // 有变化则清屏重绘
        if(last_idx != edit_idx || last_mode != edit_mode || last_ins != edit_ins)
        {
            ips200_clear();
            last_idx = edit_idx; last_mode = edit_mode; last_ins = edit_ins;
        }

        int16 sx = 0, sy = 0, sw = 240, sh = 180;
        int16 ix, iy;
        int32 *arr = edit_ins ? INS_data2 : GPS_xy_data2;

        int16 min_x = 32767, max_x = -32768;
        int16 min_y = 32767, max_y = -32768;
        for(int i = 0; i < total; i++)
        {
            ix = (int16)(arr[i] >> 16);
            iy = (int16)(arr[i] & 0xFFFF);
            if(ix < min_x) min_x = ix; if(ix > max_x) max_x = ix;
            if(iy < min_y) min_y = iy; if(iy > max_y) max_y = iy;
        }
        int16 rx = max_x - min_x; if(rx == 0) rx = 1;
        int16 ry = max_y - min_y; if(ry == 0) ry = 1;
        float sc_x = (float)sw / rx;
        float sc_y = (float)sh / ry;
        float sc = (sc_x < sc_y) ? sc_x : sc_y;
        int16 off_x = (int16)(((float)sw - rx * sc) / 2.0f);
        int16 off_y = (int16)(((float)sh - ry * sc) / 2.0f);

        for(int i = 0; i < total - 1; i++)
        {
            int16 x0 = (int16)(sx + off_x + ((int16)(arr[i]>>16) - min_x) * sc);
            int16 y0 = (int16)(sy + sh - off_y - ((int16)(arr[i]&0xFFFF) - min_y) * sc);
            int16 x1 = (int16)(sx + off_x + ((int16)(arr[i+1]>>16) - min_x) * sc);
            int16 y1 = (int16)(sy + sh - off_y - ((int16)(arr[i+1]&0xFFFF) - min_y) * sc);
            if(x0<0)x0=0; if(x0>239)x0=239; if(y0<0)y0=0; if(y0>179)y0=179;
            if(x1<0)x1=0; if(x1>239)x1=239; if(y1<0)y1=0; if(y1>179)y1=179;
            ips200_draw_line((uint16)x0, (uint16)y0, (uint16)x1, (uint16)y1,
                edit_ins ? RGB565_BLUE : RGB565_RED);
        }

        ix = (int16)(sx + off_x + ((int16)(arr[edit_idx]>>16) - min_x) * sc);
        iy = (int16)(sy + sh - off_y - ((int16)(arr[edit_idx]&0xFFFF) - min_y) * sc);
        if(ix<6)ix=6; if(ix>233)ix=233; if(iy<6)iy=6; if(iy>173)iy=173;
        ips200_draw_circle((uint16)ix, (uint16)iy, 4, RGB565_YELLOW);
    }

    // ---- 文字信息(下半屏 180~320) ----
    int32 pack = edit_ins ? INS_data2[edit_idx] : GPS_xy_data2[edit_idx];
    ips200_show_string(0, 16*12, edit_ins ? "E:INS" : "E:GPS");
    ips200_show_string(60, 16*12, "Pt:");
    ips200_show_int(80, 16*12, edit_idx, 3);
    ips200_show_string(110, 16*12, "/");
    ips200_show_int(120, 16*12, total, 3);
    ips200_show_string(150, 16*12, "Stp:");
    ips200_show_float(180, 16*12, WP_EDIT_STEP, 1, 2);

    ips200_show_string(0, 16*13, "X:");
    ips200_show_float(15, 16*13, ((int16)(pack>>16))/100.0f, 3, 2);
    if(edit_mode==1) ips200_show_string(100, 16*13, "<--");

    ips200_show_string(0, 16*14, "Y:");
    ips200_show_float(15, 16*14, ((int16)(pack&0xFFFF))/100.0f, 3, 2);
    if(edit_mode==2) ips200_show_string(100, 16*14, "<--");

    ips200_show_string(0, 16*15, "Rot:nav  Btn:edit  Sw:INS/GPS");
    ips200_show_string(120, 16*15,
        (edit_mode==0)?"BROWSE":(edit_mode==1)?"EDIT_X":"EDIT_Y");
}

// ============ KM3 菜单动作函数 ============

/************************************************
 *  函数名称: KM3_Record (菜单: 开始录线)
 ************************************************/
void KM3_Record(void)
{
    INS_Go_Point_Flag = 0;
    INS_Back_Point_Flag = 0;
    GPS_Go_Point_Flag = 0;
    GPS_Back_Point_Flag = 0;
    GPS_SetOrigin();
    INS_X = 0; INS_Y = 0;
    next = 0;
//    INS_Last_Encoder = Read_Encoder_Aggregate_Value(TIM5_ENCODER); // 左侧编码器
    INS_Last_Encoder = (Read_Encoder_Aggregate_Value(TIM5_ENCODER)+Read_Encoder_Aggregate_Value(TIM2_ENCODER))/2;// 同步编码器基准
    INS_Flag = 1;
    menu_set_status("Recording...");
}

/************************************************
 *  函数名称: KM3_SaveFlash (菜单: 存Flash)
 ************************************************/
/************************************************
 *  KM1 SaveFlash (Page 10 + Page 26)
 ************************************************/
void KM1_FlashRead(void)
{
    if(flash_check(0, 16))
    {
        flash_read_page_to_buffer(0, 16);
        INS_Resd_GoPoint_Flag  = flash_union_buffer[500].uint32_type;
        INS_Resd_BackPoint_Flag = flash_union_buffer[501].uint32_type;
        if(INS_Resd_GoPoint_Flag < 0 || INS_Resd_GoPoint_Flag > NagPoint_MAX)
            INS_Resd_GoPoint_Flag = 0;
        if(INS_Resd_BackPoint_Flag < 0 || INS_Resd_BackPoint_Flag > NagPoint_MAX)
            INS_Resd_BackPoint_Flag = 0;
        int32 total = INS_Resd_GoPoint_Flag + INS_Resd_BackPoint_Flag;
        for(int i = 0; i < total; i++)
            INS_data2[i] = flash_union_buffer[i].uint32_type;
    }
    else { INS_Resd_GoPoint_Flag = 0; INS_Resd_BackPoint_Flag = 0; }
}

void KM1_GPSRead(void)
{
    if(flash_check(0, 17))
    {
        flash_read_page_to_buffer(0, 17);
        GPS_Resd_GoPoint_Flag  = flash_union_buffer[500].uint32_type;
        GPS_Resd_BackPoint_Flag = flash_union_buffer[501].uint32_type;
        if(GPS_Resd_GoPoint_Flag < 0 || GPS_Resd_GoPoint_Flag > NagPoint_MAX)
            GPS_Resd_GoPoint_Flag = 0;
        if(GPS_Resd_BackPoint_Flag < 0 || GPS_Resd_BackPoint_Flag > NagPoint_MAX)
            GPS_Resd_BackPoint_Flag = 0;
        int32 total = GPS_Resd_GoPoint_Flag + GPS_Resd_BackPoint_Flag;
        for(int i = 0; i < total; i++)
            GPS_xy_data2[i] = flash_union_buffer[i].uint32_type;
    }
    else { GPS_Resd_GoPoint_Flag = 0; GPS_Resd_BackPoint_Flag = 0; }
}

void KM1_SaveFlash(void)
{
    flash_buffer_clear();
    flash_union_buffer[500].uint32_type = INS_Go_Point_Flag;
    flash_union_buffer[501].uint32_type = INS_Back_Point_Flag;
    for(int i = 0; i < (INS_Go_Point_Flag + INS_Back_Point_Flag); i++)
        flash_union_buffer[i].uint32_type = INS_data2[i];
    if(flash_check(0, 16))
        flash_erase_page(0, 16);
    flash_write_page_from_buffer(0, 16);

    flash_buffer_clear();
    flash_union_buffer[500].uint32_type = GPS_Go_Point_Flag;
    flash_union_buffer[501].uint32_type = GPS_Back_Point_Flag;
    for(int i = 0; i < (GPS_Go_Point_Flag + GPS_Back_Point_Flag); i++)
        flash_union_buffer[i].uint32_type = GPS_xy_data2[i];
    if(flash_check(0, 17))
        flash_erase_page(0, 17);
    flash_write_page_from_buffer(0, 17);

    INS_Flag = 0;
    menu_set_status("KM1 Saved to Flash");
    Buzzer_check(50);
}

void KM1_LoadFlash(void)
{
    KM1_FlashRead();
    KM1_GPSRead();
    next = 0;
    INS_Flag = 0;
    menu_set_status("KM1 Loaded from Flash");
    Buzzer_check(50);
}

void KM3_SaveFlash(void)
{
    /* === 合并保存: 一次擦除, 两次写入, 避免互相覆盖 === */

    /* 第1步: 写入 INS 到 Page 9 */
    flash_buffer_clear();
    flash_union_buffer[500].uint32_type = INS_Go_Point_Flag;
    flash_union_buffer[501].uint32_type = INS_Back_Point_Flag;
    for(int i = 0; i < (INS_Go_Point_Flag + INS_Back_Point_Flag); i++)
        flash_union_buffer[i].uint32_type = INS_data2[i];

    if(flash_check(0, 9))
        flash_erase_page(0, 9);          /* 擦除 Page 9 所在扇区 (flag=1) */
    flash_write_page_from_buffer(0, 9);  /* 写入 INS (flag 复位为 0) */

    /* 第2步: 写入 GPS 到 Page 25 (不同扇区, 互不影响) */
    flash_buffer_clear();
    flash_union_buffer[500].uint32_type = GPS_Go_Point_Flag;
    flash_union_buffer[501].uint32_type = GPS_Back_Point_Flag;
    for(int i = 0; i < (GPS_Go_Point_Flag + GPS_Back_Point_Flag); i++)
        flash_union_buffer[i].uint32_type = GPS_xy_data2[i];

    if(flash_check(0, 25))
        flash_erase_page(0, 25);          /* 擦除 Page 25 所在扇区 (独立扇区) */
    flash_write_page_from_buffer(0, 25);  /* 写入 GPS */

    INS_Flag = 0;
    menu_set_status("Saved to Flash");
    Buzzer_check(50);
}

/************************************************
 *  函数名称: KM3_LoadFlash (菜单: 读Flash)
 ************************************************/
void KM3_LoadFlash(void)
{
    INS_FlashRead();       /* 读 Page 9 */
    GPS_FlashRead();       /* 读 Page 25 */
    next = 0;
    INS_Flag = 0;
    menu_set_status("Loaded from Flash");
    Buzzer_check(50);
}

/************************************************
 *  函数名称: KM3_Run (菜单: 发车)
 ************************************************/
void KM1_Run_INS(void)
{
    next = 0;
    stuck_waypoint = -1;
    stuck_counter = 0;
    GPS_SetOrigin();
    ips200_clear();
    dispatch_car = 1;
    INS_Flag = 2;
    Back_Motor_Set_Speed = km3_fwd_speed / 2;
    Back_Motor_SetPwm(200, 200);
    menu_set_status("KM1 Running... Long K1 to stop");
    Buzzer_check(50);
}

void KM3_Run(void)
{
    next = 0;
    stuck_waypoint = -1;
    stuck_counter = 0;
    GPS_SetOrigin();
    ips200_clear();             // 清屏,避免菜单残留
    dispatch_car = 1;
    INS_Flag = 2;
    Back_Motor_Set_Speed = km3_fwd_speed / 2;  // half fwd for launch
    Back_Motor_SetPwm(200, 200);
    menu_set_status("Running... Long press K1 to stop");
    Buzzer_check(50);
}

/************************************************
 *  函数名称: KM3_Stop (菜单: 停车)//改成差速程度调节，key1停车
 ************************************************/
void KM3_Stop(void)
{
    ips200_clear();
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    ips200_show_string(0, 16*0,  "DIFF GAIN");
    ips200_show_string(0, 16*18, "Long K1:Exit");

    while(1)
    {
        system_delay_ms(10);
        menu_key_scanner();
        menu_encoder_update();
        if(MENU_KEY_LONG_PRESS == menu_key_get_state(MENU_KEY_1))
        {
            menu_key_clear_state(MENU_KEY_1);
            break;
        }
        if(menu_encoder_has_change())
        {
            int enc = menu_encoder_get_change();
            diff_gain += enc * 0.05f;
            if(diff_gain < 0.0f)   diff_gain = 0.0f;
            if(diff_gain > 1.00f)  diff_gain = 1.00f;
        }
        ips200_show_float(0, 16*1, diff_gain, 2, 1);
    }

    ips200_clear();
    menu_set_status("diff_gain set");
    Buzzer_check(50);
}

/************************************************
 *  函数名称: KM3_ReversePath (菜单: 反转路径)
 *  功能说明：将INS路径倒序，前进路变返回路，返回路变前进路
 ************************************************/
void KM3_ReversePath(void)
{
    int32 total = INS_Resd_GoPoint_Flag + INS_Resd_BackPoint_Flag;
    if(total == 0) { menu_set_status("No data! Load Flash first"); return; }

    reverse_path_xy(INS_data2, total);

    /* 反转后全部为前进段: 车头向前从车库开回起点, 不后退 */
    INS_Resd_GoPoint_Flag  = total;
    INS_Resd_BackPoint_Flag = 0;

    /* 同步到录点计数，确保保存时正确 */
    INS_Go_Point_Flag  = total;
    INS_Back_Point_Flag = 0;

    menu_set_status("Path Reversed (All Go)");
    Buzzer_check(50);
}

/************************************************
 *  函数名称: KM3_Display (菜单: 画路径图)
 ************************************************/
void KM3_SpeedSet(void)
{
    ips200_full(RGB565_BLACK);
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    ips200_show_string(0, 16*18, "Enc:Adj Btn:Switch LongK1:Exit");

    static uint8 sel = 0;  /* 0=fwd, 1=rev */
    while(1)
    {
        system_delay_ms(10);
        menu_key_scanner();
        if(MENU_KEY_LONG_PRESS == menu_key_get_state(MENU_KEY_1))
        {
            menu_key_clear_state(MENU_KEY_1);
            break;
        }

        menu_encoder_update();
        if(menu_encoder_has_change())
        {
            int enc = menu_encoder_get_change();
            int32 step = 50;
            if(sel == 0) { km3_fwd_speed += enc * step; if(km3_fwd_speed < 100) km3_fwd_speed = 100; if(km3_fwd_speed > 1000) km3_fwd_speed = 1000; }
            else         { km3_rev_speed  += enc * step; if(km3_rev_speed  < 100) km3_rev_speed  = 100; if(km3_rev_speed  > 600)  km3_rev_speed  = 600;  }
        }

        /* Encoder button: toggle selection */
        static uint8 last_btn = 1;
        uint8 btn_now = gpio_get_level(P11_11);
        if(last_btn == 1 && btn_now == 0) { sel = !sel; }
        last_btn = btn_now;

        ips200_show_string(0, 16*1, "SPEED SET");
        ips200_show_string(0, 16*3, "FWD:");
        ips200_show_int(50, 16*3, km3_fwd_speed, 4);
        ips200_show_string(100, 16*3, (sel == 0) ? "<--" : "   ");
        ips200_show_string(0, 16*4, "REV:");
        ips200_show_int(50, 16*4, km3_rev_speed, 4);
        ips200_show_string(100, 16*4, (sel == 1) ? "<--" : "   ");
    }

    ips200_clear();
    menu_set_status("Speed saved");
    Buzzer_check(50);
}
void KM3_Correct(void)
{
    GPS_Correct_INS();
    menu_set_status("INS corrected by GPS");
}

/************************************************
 *  函数名称: KM3_Replace (菜单: GPS替换INS)
 ************************************************/
void KM3_Replace(void)
{
    GPS_Replace_INS();
    menu_set_status("INS replaced by GPS");
}

/************************************************
 *  函数名称: KM3_Gain (菜单: 调GPS融合权重)
 ************************************************/
void KM3_Gain(void)
{
    ips200_full(RGB565_BLACK);
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    ips200_show_string(0, 16*18, "Long K1:Exit");

    while(1)
    {
        system_delay_ms(10);
        menu_key_scanner();
        menu_encoder_update();          // 必须调用才能读到编码器
        if(MENU_KEY_LONG_PRESS == menu_key_get_state(MENU_KEY_1))
        {
            menu_key_clear_state(MENU_KEY_1);
            break;
        }
        // 读取菜单编码器
        if(menu_encoder_has_change())
        {
            int enc = menu_encoder_get_change();
            gps_fusion_gain += enc * 0.01f;
            if(gps_fusion_gain < 0.0f)  gps_fusion_gain = 0.0f;
            if(gps_fusion_gain > 1.0f)  gps_fusion_gain = 1.0f;
        }
        // 短按跳变: 0->1 或 1->0
        if(MENU_KEY_SHORT_PRESS == menu_key_get_state(MENU_KEY_1))
        {
            menu_key_clear_state(MENU_KEY_1);
            gps_fusion_gain = (gps_fusion_gain < 0.5f) ? 1.0f : 0.0f;
        }
        // 显示
        ips200_show_string(0, 16*4, "GPS Fusion Gain");
        ips200_show_string(0, 16*6, "Gain:");
        ips200_show_float(60, 16*6, gps_fusion_gain, 1, 2);
        ips200_show_string(0, 16*7, "(0=INS only, 1=GPS only)");
        ips200_show_string(0, 16*9, "Rot:+/-0.01  Btn:0/1");
    }
    menu_set_status("Gain updated");
}

/************************************************
 *  函数名称: KM3_GPSMode (菜单: GPS模式切换)
 ************************************************/
void KM3_GPSMode(void)
{
    gps_confirm_mode = !gps_confirm_mode;
    if(gps_confirm_mode == 0)
        menu_set_status("Mode: Pure INS");
    else
        menu_set_status("Mode: INS+GPS");
    Buzzer_check(50);
}

/************************************************
 *  函数名称: KM3_Editor (菜单: 路点编辑器)
 ************************************************/
void KM3_DisplayPath(void)
{
    int32 total = INS_Resd_GoPoint_Flag + INS_Resd_BackPoint_Flag;
    if(total == 0) { menu_set_status("No data! Load first"); return; }

    ips200_full(RGB565_BLACK);
    ips200_show_string(0, 16*18, "Blue:INS Red:GPS LongK1:Exit");

    /* Draw INS path in blue */
    {
        int16 sx = 0, sy = 0, sw = 240, sh = 170;
        int16 ix, iy;
        int16 min_x = 32767, max_x = -32768, min_y = 32767, max_y = -32768;
        int32 *arr = INS_data2;
        for(int i = 0; i < total; i++)
        {
            ix = (int16)(arr[i] >> 16); iy = (int16)(arr[i] & 0xFFFF);
            if(ix < min_x) min_x = ix; if(ix > max_x) max_x = ix;
            if(iy < min_y) min_y = iy; if(iy > max_y) max_y = iy;
        }
        int16 rx = max_x - min_x; if(rx == 0) rx = 1;
        int16 ry = max_y - min_y; if(ry == 0) ry = 1;
        float sc = ((float)sw / rx < (float)sh / ry) ? (float)sw / rx : (float)sh / ry;
        int16 off_x = (int16)(((float)sw - rx * sc) / 2.0f);
        int16 off_y = (int16)(((float)sh - ry * sc) / 2.0f);
        for(int i = 0; i < total - 1; i++)
        {
            int16 x0 = (int16)(sx + off_x + ((int16)(arr[i]>>16) - min_x) * sc);
            int16 y0 = (int16)(sy + sh - off_y - ((int16)(arr[i]&0xFFFF) - min_y) * sc);
            int16 x1 = (int16)(sx + off_x + ((int16)(arr[i+1]>>16) - min_x) * sc);
            int16 y1 = (int16)(sy + sh - off_y - ((int16)(arr[i+1]&0xFFFF) - min_y) * sc);
            if(x0<0)x0=0; if(x0>239)x0=239; if(y0<0)y0=0; if(y0>179)y0=179;
            if(x1<0)x1=0; if(x1>239)x1=239; if(y1<0)y1=0; if(y1>179)y1=179;
            ips200_draw_line((uint16)x0, (uint16)y0, (uint16)x1, (uint16)y1, RGB565_BLUE);
        }
    }

    /* Draw GPS path in red */
    int32 gps_total = GPS_Resd_GoPoint_Flag + GPS_Resd_BackPoint_Flag;
    if(gps_total > 0)
    {
        int16 sx = 0, sy = 0, sw = 240, sh = 170;
        int16 ix, iy;
        int16 min_x = 32767, max_x = -32768, min_y = 32767, max_y = -32768;
        int32 *arr = GPS_xy_data2;
        for(int i = 0; i < gps_total; i++)
        {
            ix = (int16)(arr[i] >> 16); iy = (int16)(arr[i] & 0xFFFF);
            if(ix < min_x) min_x = ix; if(ix > max_x) max_x = ix;
            if(iy < min_y) min_y = iy; if(iy > max_y) max_y = iy;
        }
        int16 rx = max_x - min_x; if(rx == 0) rx = 1;
        int16 ry = max_y - min_y; if(ry == 0) ry = 1;
        float sc = ((float)sw / rx < (float)sh / ry) ? (float)sw / rx : (float)sh / ry;
        int16 off_x = (int16)(((float)sw - rx * sc) / 2.0f);
        int16 off_y = (int16)(((float)sh - ry * sc) / 2.0f);
        for(int i = 0; i < gps_total - 1; i++)
        {
            int16 x0 = (int16)(sx + off_x + ((int16)(arr[i]>>16) - min_x) * sc);
            int16 y0 = (int16)(sy + sh - off_y - ((int16)(arr[i]&0xFFFF) - min_y) * sc);
            int16 x1 = (int16)(sx + off_x + ((int16)(arr[i+1]>>16) - min_x) * sc);
            int16 y1 = (int16)(sy + sh - off_y - ((int16)(arr[i+1]&0xFFFF) - min_y) * sc);
            if(x0<0)x0=0; if(x0>239)x0=239; if(y0<0)y0=0; if(y0>179)y0=179;
            if(x1<0)x1=0; if(x1>239)x1=239; if(y1<0)y1=0; if(y1>179)y1=179;
            ips200_draw_line((uint16)x0, (uint16)y0, (uint16)x1, (uint16)y1, RGB565_RED);
        }
    }

    while(1)
    {
        system_delay_ms(10);
        menu_key_scanner();
        if(MENU_KEY_LONG_PRESS == menu_key_get_state(MENU_KEY_1))
        {
            menu_key_clear_state(MENU_KEY_1);
            break;
        }
    }
    ips200_clear();
}

void KM3_Editor(void)
{
    static int16 edit_idx = 0;
    static uint8 edit_mode = 0;
    static uint8 edit_ins = 1;
    static uint8 first_run = 1;
    int32 total = INS_Resd_GoPoint_Flag + INS_Resd_BackPoint_Flag;

    if(total == 0) { menu_set_status("No data! Load Flash first"); return; }

    int8 enc_acc = 0;
    edit_mode = 0;  // 每次进入都从浏览模式开始
    Enc_Button_Short();  // 清空上次遗留的按键边沿，防止误跳EDIT_X
    if(first_run)
    {
        Enc_GPIO_Init();
        edit_idx = 0; enc_btn_press_time = 0;
        memset(waypoint_mark, 0, sizeof(waypoint_mark)); mark_count = 0;
        first_run = 0;
    }

    ips200_full(RGB565_BLACK);
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    ips200_show_string(0, 16*18, "K1:Ins K2:Sav K3:Mrk K4:Clr");

    while(1)
    {
        system_delay_ms(10);
        menu_key_scanner();
        if(MENU_KEY_LONG_PRESS == menu_key_get_state(MENU_KEY_1))
        {
            menu_key_clear_state(MENU_KEY_1);
            first_run = 1;
            break;
        }

        // 编码器
        enc_acc += enc_accumulator; enc_accumulator = 0;
        uint8 btn = Enc_Button_Short();

        // 数据集切换: KEY1 在 INS(惯导) / GPS 之间切换
        if(key1_flag) { key1_flag = 0; edit_ins = !edit_ins; }

        // KEY2: 保存修改到Flash
        if(key2_flag) { key2_flag = 0; KM3_SaveFlash(); menu_set_status("Saved to Flash"); Buzzer_check(50); }

        // KEY3: 切换当前点标记
        if(key3_flag) { key3_flag = 0;
            if(waypoint_mark[edit_idx]) { waypoint_mark[edit_idx] = 0; mark_count--; }
            else                         { waypoint_mark[edit_idx] = 1; mark_count++; }
            Buzzer_check(30);
        }

        // KEY4: 清除全部标记
        if(key4_flag) { key4_flag = 0;
            memset(waypoint_mark, 0, sizeof(waypoint_mark)); mark_count = 0;
            Buzzer_check(30);
        }

        // 状态机
        if(edit_mode == 0)  // BROWSE
        {
            if(abs(enc_acc) >= 2)
            {
                int8 dir = (enc_acc > 0) ? 1 : -1;
                edit_idx += dir;
                if(edit_idx < 0) edit_idx = total - 1;
                if(edit_idx >= total) edit_idx = 0;
                enc_acc = 0;
            }
            if(btn == 1) { edit_mode = 1; enc_acc = 0; }
        }
        else if(edit_mode == 1)  // EDIT_X
        {
            if(abs(enc_acc) >= 2)
            {
                float step = WP_EDIT_STEP * 100.0f;
                int8 dir = (enc_acc > 0) ? 1 : -1;
                int32 *arr = edit_ins ? INS_data2 : GPS_xy_data2;
                if(waypoint_mark[edit_idx] && mark_count > 0)
                {
                    /* 批量修点: 所有标记点同步偏移 X */
                    for(int i = 0; i < total; i++)
                    {
                        if(waypoint_mark[i])
                        {
                            int16 x = (int16)(arr[i] >> 16) + (int16)(dir * step);
                            int16 y = (int16)(arr[i] & 0xFFFF);
                            arr[i] = (((uint32)(uint16)x) << 16) | ((uint16)y);
                        }
                    }
                }
                else
                {
                    /* 单点修点 */
                    int16 x = (int16)(arr[edit_idx] >> 16) + (int16)(dir * step);
                    int16 y = (int16)(arr[edit_idx] & 0xFFFF);
                    arr[edit_idx] = (((uint32)(uint16)x) << 16) | ((uint16)y);
                }
                enc_acc = 0;
            }
            if(btn == 1) { edit_mode = 2; enc_acc = 0; }
        }
        else if(edit_mode == 2)  // EDIT_Y
        {
            if(abs(enc_acc) >= 2)
            {
                float step = WP_EDIT_STEP * 100.0f;
                int8 dir = (enc_acc > 0) ? 1 : -1;
                int32 *arr = edit_ins ? INS_data2 : GPS_xy_data2;
                if(waypoint_mark[edit_idx] && mark_count > 0)
                {
                    /* 批量修点: 所有标记点同步偏移 Y */
                    for(int i = 0; i < total; i++)
                    {
                        if(waypoint_mark[i])
                        {
                            int16 x = (int16)(arr[i] >> 16);
                            int16 y = (int16)(arr[i] & 0xFFFF) + (int16)(dir * step);
                            arr[i] = (((uint32)(uint16)x) << 16) | ((uint16)y);
                        }
                    }
                }
                else
                {
                    /* 单点修点 */
                    int16 x = (int16)(arr[edit_idx] >> 16);
                    int16 y = (int16)(arr[edit_idx] & 0xFFFF) + (int16)(dir * step);
                    arr[edit_idx] = (((uint32)(uint16)x) << 16) | ((uint16)y);
                }
                enc_acc = 0;
            }
            if(btn == 1) { edit_mode = 0; enc_acc = 0; }
        }

        // 画路径
        {
            static int16 last_idx = -1, last_mode = -1;
            static uint8 last_ins = 255;
            if(last_idx != edit_idx || last_mode != edit_mode || last_ins != edit_ins)
            {
                ips200_clear(); last_idx = edit_idx; last_mode = edit_mode; last_ins = edit_ins;
            }

            int32 *arr = edit_ins ? INS_data2 : GPS_xy_data2;
            int16 sx = 0, sy = 0, sw = 240, sh = 180;
            int16 ix, iy;
            int16 min_x = 32767, max_x = -32768, min_y = 32767, max_y = -32768;
            for(int i = 0; i < total; i++)
            {
                ix = (int16)(arr[i] >> 16); iy = (int16)(arr[i] & 0xFFFF);
                if(ix < min_x) min_x = ix; if(ix > max_x) max_x = ix;
                if(iy < min_y) min_y = iy; if(iy > max_y) max_y = iy;
            }
            int16 rx = max_x - min_x; if(rx == 0) rx = 1;
            int16 ry = max_y - min_y; if(ry == 0) ry = 1;
            float sc_x = (float)sw / rx, sc_y = (float)sh / ry, sc = (sc_x < sc_y) ? sc_x : sc_y;
            int16 off_x = (int16)(((float)sw - rx * sc) / 2.0f), off_y = (int16)(((float)sh - ry * sc) / 2.0f);

            for(int i = 0; i < total - 1; i++)
            {
                int16 x0 = (int16)(sx + off_x + ((int16)(arr[i]>>16) - min_x) * sc);
                int16 y0 = (int16)(sy + sh - off_y - ((int16)(arr[i]&0xFFFF) - min_y) * sc);
                int16 x1 = (int16)(sx + off_x + ((int16)(arr[i+1]>>16) - min_x) * sc);
                int16 y1 = (int16)(sy + sh - off_y - ((int16)(arr[i+1]&0xFFFF) - min_y) * sc);
                if(x0<0)x0=0; if(x0>239)x0=239; if(y0<0)y0=0; if(y0>179)y0=179;
                if(x1<0)x1=0; if(x1>239)x1=239; if(y1<0)y1=0; if(y1>179)y1=179;
                ips200_draw_line((uint16)x0, (uint16)y0, (uint16)x1, (uint16)y1,
                    edit_ins ? RGB565_BLUE : RGB565_RED);
            }
            /* 画所有标记点的红色圆圈 */
            if(mark_count > 0)
            {
                for(int i = 0; i < total; i++)
                {
                    if(waypoint_mark[i])
                    {
                        int16 mx = (int16)(sx + off_x + ((int16)(arr[i]>>16) - min_x) * sc);
                        int16 my = (int16)(sy + sh - off_y - ((int16)(arr[i]&0xFFFF) - min_y) * sc);
                        if(mx<7)mx=7; if(mx>232)mx=232;
                        if(my<7)my=7; if(my>172)my=172;
                        ips200_draw_circle((uint16)mx, (uint16)my, 6, RGB565_RED);
                    }
                }
            }
            /* 画选中点的黄色圆圈 (覆盖在最上层) */
            ix = (int16)(sx + off_x + ((int16)(arr[edit_idx]>>16) - min_x) * sc);
            iy = (int16)(sy + sh - off_y - ((int16)(arr[edit_idx]&0xFFFF) - min_y) * sc);
            if(ix<6)ix=6; if(ix>233)ix=233; if(iy<6)iy=6; if(iy>173)iy=173;
            ips200_draw_circle((uint16)ix, (uint16)iy, 4, RGB565_YELLOW);
        }

        // 文字
        int32 pack = edit_ins ? INS_data2[edit_idx] : GPS_xy_data2[edit_idx];
        ips200_show_string(0, 16*12, edit_ins ? "E:INS" : "E:GPS");
        ips200_show_string(60, 16*12, "Pt:");
        ips200_show_int(80, 16*12, edit_idx, 3);
        ips200_show_string(110, 16*12, "/");
        ips200_show_int(120, 16*12, total, 3);
        ips200_show_string(150, 16*12, "Stp:");
        ips200_show_float(180, 16*12, WP_EDIT_STEP, 1, 2);
        ips200_show_string(0, 16*13, "X:");
        ips200_show_float(15, 16*13, ((int16)(pack>>16))/100.0f, 3, 2);
        if(edit_mode==1) ips200_show_string(100, 16*13, "<--");
        ips200_show_string(0, 16*14, "Y:");
        ips200_show_float(15, 16*14, ((int16)(pack&0xFFFF))/100.0f, 3, 2);
        if(edit_mode==2) ips200_show_string(100, 16*14, "<--");
        ips200_show_string(120, 16*15,
            (edit_mode==0)?"BROWSE":(edit_mode==1)?"EDIT_X":"EDIT_Y");
        /* 标记状态 */
        ips200_show_string(0, 16*16, mark_count > 0 ? "M:" : "  ");
        if(mark_count > 0) ips200_show_int(20, 16*16, mark_count, 2);
        if(mark_count > 0 && waypoint_mark[edit_idx])
            ips200_show_string(0, 16*17, "Batch Edit ON");
        else if(mark_count > 0)
            ips200_show_string(0, 16*17, "Batch Edit OFF");
    }
    menu_set_status("Editor closed");
}


/************************************************
 *  函数名称: KM2_WaypointEditor (KM2 GPS路点编辑器, 模仿KM3修点模式)
 *  功能说明：编码器浏览/编辑16个GPS路点的经纬度
 *  操作说明：
 *    旋转编码器 → 浏览模式选点 / 编辑模式调值
 *    短按编码器 → BROWSE → EDIT_LAT → EDIT_LOG → BROWSE
 *    KEY2      → 保存到Flash
 *    KEY3      → 打标记(红圈) / 取消标记
 *    KEY4      → 清除全部标记
 *    长按编码器 → 退出
 ************************************************/
void KM2_WaypointEditor(void)
{
    static int16 edit_idx = 0;
    static uint8 edit_mode = 0;       /* 0=BROWSE, 1=EDIT_X, 2=EDIT_Y */
    static int8 enc_acc = 0;
    static uint8 first_run = 1;
    static uint8 show_ins = 0;  /* 0=GPS, 1=INS */
    const int16 total = 16;

    edit_mode = 0;  // 每次进入都从浏览模式开始
    Enc_Button_Short();  // 清空上次遗留的按键边沿
    if(first_run)
    {
        Enc_GPIO_Init();
        edit_idx = 0; enc_acc = 0; enc_btn_press_time = 0; first_run = 0;
    }

    ips200_full(RGB565_BLACK);
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    ips200_show_string(0, 16*17, "K1:INS/GPS K2:Sav K3:Mrk");
    ips200_show_string(0, 16*18, "K4:Clr LongK1:BK");

    while(1)
    {
        system_delay_ms(10);
        menu_key_scanner();
        if(MENU_KEY_LONG_PRESS == menu_key_get_state(MENU_KEY_1))
        {
            menu_key_clear_state(MENU_KEY_1);
            first_run = 1;
            KM2_edit_active = 0;
            km2_submode = KM2_SUBMODE_OFF;
            menu_system_mode = 0;
            ips200_clear();
            menu_set_status("KM2 Editor closed");
            return;
        }

        /* 编码器累积 */
        enc_acc += enc_accumulator; enc_accumulator = 0;
        uint8 btn = Enc_Button_Short();

                /* K1: 切换GPS/INS显示 */
        if(key1_flag) { key1_flag = 0; show_ins = !show_ins; }

/* KEY2: 保存到Flash */
        if(key2_flag) { key2_flag = 0;
            km2_xy2de();
            Up_flash_point_2();
            FLASH_Get_KM2();
            menu_set_status("KM2 Saved to Flash"); Buzzer_check(50);
        }

        /* KEY3: 打标记 */
        if(key3_flag) { key3_flag = 0;
            if(waypoint_mark[edit_idx]) { waypoint_mark[edit_idx] = 0; mark_count--; }
            else                         { waypoint_mark[edit_idx] = 1; mark_count++; }
            Buzzer_check(30);
        }

        /* KEY4: 清除全部标记 */
        if(key4_flag) { key4_flag = 0;
            memset(waypoint_mark, 0, sizeof(waypoint_mark)); mark_count = 0;
            Buzzer_check(30);
        }

        /* ====== 状态机 ====== */
        if(edit_mode == 0)  /* BROWSE */
        {
            if(abs(enc_acc) >= 2)
            {
                int8 dir = (enc_acc > 0) ? 1 : -1;
                edit_idx += dir;
                if(edit_idx < 0) edit_idx = total - 1;
                if(edit_idx >= total) edit_idx = 0;
                enc_acc = 0;
            }
            if(btn == 1) { edit_mode = 1; enc_acc = 0; }
        }
        else if(edit_mode == 1)  /* EDIT_LAT */
        {
            if(abs(enc_acc) >= 2)
            {
                float step = 0.01f;          /* 1cm */
                int8 dir = (enc_acc > 0) ? 1 : -1;
                if(waypoint_mark[edit_idx] && mark_count > 0)
                {
                    for(int i = 0; i < total; i++)
                        if(waypoint_mark[i]) KM2_GPS_Y[i] += dir * step;
                }
                else
                    KM2_GPS_Y[edit_idx] += dir * step;
                enc_acc = 0;
            }
            if(btn == 1) { edit_mode = 2; enc_acc = 0; }
        }
        else if(edit_mode == 2)  /* EDIT_LOG */
        {
            if(abs(enc_acc) >= 2)
            {
                float step = 0.01f;
                int8 dir = (enc_acc > 0) ? 1 : -1;
                if(waypoint_mark[edit_idx] && mark_count > 0)
                {
                    for(int i = 0; i < total; i++)
                        if(waypoint_mark[i]) KM2_GPS_X[i] += dir * step;
                }
                else
                    KM2_GPS_X[edit_idx] += dir * step;
                enc_acc = 0;
            }
            if(btn == 1) { edit_mode = 0; enc_acc = 0; }
        }

        /* ====== 画路径 ====== */
        {
            static int16 last_idx = -1, last_mode = -1;
            if(last_idx != edit_idx || last_mode != edit_mode)
            {
                ips200_clear(); last_idx = edit_idx; last_mode = edit_mode;
            }

            float min_x = KM2_GPS_X[0], max_x = KM2_GPS_X[0];
            float min_y = KM2_GPS_Y[0], max_y = KM2_GPS_Y[0];
            for(int i = 1; i < total; i++)
            {
                if(KM2_GPS_X[i] < min_x) min_x = KM2_GPS_X[i];
                if(KM2_GPS_X[i] > max_x) max_x = KM2_GPS_X[i];
                if(KM2_GPS_Y[i] < min_y) min_y = KM2_GPS_Y[i];
                if(KM2_GPS_Y[i] > max_y) max_y = KM2_GPS_Y[i];
            }
            float rx = max_x - min_x; if(rx < 0.01f) rx = 0.01f;
            float ry = max_y - min_y; if(ry < 0.01f) ry = 0.01f;

            int16 sx = 5, sy = 0, sw = 230, sh = 175;
            float sc = (sw / rx < sh / ry) ? sw / rx : sh / ry;

            for(int i = 0; i < total - 1; i++)
            {
                int16 x0 = (int16)(sx + (KM2_GPS_X[i]   - min_x) * sc);
                int16 y0 = (int16)(sy + sh - (KM2_GPS_Y[i]   - min_y) * sc);
                int16 x1 = (int16)(sx + (KM2_GPS_X[i+1] - min_x) * sc);
                int16 y1 = (int16)(sy + sh - (KM2_GPS_Y[i+1] - min_y) * sc);
                if(x0<0)x0=0; if(x0>239)x0=239; if(y0<0)y0=0; if(y0>179)y0=179;
                if(x1<0)x1=0; if(x1>239)x1=239; if(y1<0)y1=0; if(y1>179)y1=179;
                ips200_draw_line((uint16)x0, (uint16)y0, (uint16)x1, (uint16)y1, RGB565_GREEN);
            }

            for(int i = 0; i < total; i++)
            {
                int16 px = (int16)(sx + (KM2_GPS_X[i] - min_x) * sc);
                int16 py = (int16)(sy + sh - (KM2_GPS_Y[i] - min_y) * sc);
                if(px<3)px=3; if(px>236)px=236; if(py<3)py=3; if(py>176)py=176;
                if(waypoint_mark[i])
                    ips200_draw_circle((uint16)px, (uint16)py, 3, RGB565_RED);
                else
                    ips200_draw_circle((uint16)px, (uint16)py, 1, RGB565_WHITE);
            }

            {
                int16 cx = (int16)(sx + (KM2_GPS_X[edit_idx] - min_x) * sc);
                int16 cy = (int16)(sy + sh - (KM2_GPS_Y[edit_idx] - min_y) * sc);
                if(cx<6)cx=6; if(cx>233)cx=233; if(cy<6)cy=6; if(cy>173)cy=173;
                ips200_draw_circle((uint16)cx, (uint16)cy, 5, RGB565_YELLOW);
                ips200_draw_circle((uint16)cx, (uint16)cy, 6, RGB565_YELLOW);
            }
        }
        /* ====== 文字信息 ====== */
        ips200_show_string(0, 16*12, edit_mode==0?(show_ins?"BROWSE INS":"BROWSE GPS"):edit_mode==1?"EDIT_Y":"EDIT_X");
        ips200_show_string(80, 16*12, "Pt:");
        ips200_show_int(100, 16*12, edit_idx, 2);
        ips200_show_string(130, 16*12, "/16");
        ips200_show_string(0, 16*13, "X:");
        ips200_show_float(20, 16*13, KM2_GPS_X[edit_idx], 3, 2);
        ips200_show_string(90, 16*13, "m");
        if(edit_mode==2) ips200_show_string(110, 16*13, "<--");
        ips200_show_string(0, 16*14, "Y:");
        ips200_show_float(20, 16*14, KM2_GPS_Y[edit_idx], 3, 2);
        ips200_show_string(90, 16*14, "m");
        if(edit_mode==1) ips200_show_string(110, 16*14, "<--");
        ips200_show_string(0, 16*15, "Step:1cm");

        ips200_show_string(120, 16*15, mark_count > 0 ? "M:" : "  ");
        if(mark_count > 0) ips200_show_int(140, 16*15, mark_count, 2);
        if(mark_count > 0 && waypoint_mark[edit_idx])
            ips200_show_string(0, 16*17, "Batch Edit ON");
        else if(mark_count > 0)
            ips200_show_string(0, 16*17, "Batch Edit OFF");
    }
}
