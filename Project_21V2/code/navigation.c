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

uint8 Camera_Navigation_Motor_flag = 0; //摄像头与惯导切换电机控制标志位（1惯导控制，0摄像头控制）

float Yaw_Start = 0;          // 当前段起始Yaw
float Yaw_End = 0;            // 当前段结束Yaw
float Yaw_Target = 0;         // 插值后的目标Yaw
float Last_Yaw_Target = 0;    // 上一次目标Yaw

#define YAW_MAX_CHANGE   20.0f // 最大Yaw变化限制

// ---- Yaw映射（导航坐标系） ----
float yaw_offset = 0.0f;         // 陀螺仪到映射的偏移量（extern可访问）
double Yaw_Mapped = 0.0f;         // 映射后的航向角（0~360°）


float INS_X_MH = 0.0f;      // 前进目标X坐标(m)
float INS_Y_MH = 2.6f;      // 前进目标Y坐标(m)
uint8 INS_mh_phase = 0;      // MH阶段：0=前进到目标，1=返回原点



void Set_Map_Yaw(float target)
{
    // target: 你想映射到的角度，0~360
    yaw_offset = target - (float)Yaw;
}

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

    Read_Continue_Encoder_Left=Read_Encoder_Aggregate_Value(TIM5_ENCODER);
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
       Angle_Target_Position = -GPS_IMU_PID_Start(Open,angle_diff(Yaw_Target,Yaw),0);
       Back_Motor_SetPwm(300,300);
   }
   else
   {
       // 后退
       Angle_Target_Position =GPS_IMU_PID_Start(Open,angle_diff(Yaw_Target,Yaw),0);
       Back_Motor_SetPwm(-300,-300);
   }

   //================================================//




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
    static int32_t last_encoder = 0;

    //--------------------------
    // 读取编码器
    //--------------------------

    int32_t now_encoder;

    now_encoder =
        Read_Encoder_Aggregate_Value(
            TIM5_ENCODER);

    // 本周期编码器增量
    int32_t delta_encoder;

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

// 6条线路支持
int32 current_route_index = 0;  // 当前线路索引（0-5）


/************************************************************
 * 函数名称: 纯编码器和陀螺仪转二维坐标函数
 * 功能说明: 利用编码器计算10ms内移动距离，
 *           利用前后两次Yaw的中值进行二维坐标积分
 * 参数说明: 无
 * 函数返回: 无
 * 修改时间: 2026年8月11日
 * 备注: 放中断执行，10ms
 ************************************************************/
float INS_X = 0.0f;          // X坐标(m)
float INS_Y = 0.0f;          // Y坐标(m)
float INS_Yaw = 0.0f;        // 当前航向角(rad)



int32 INS_Last_Encoder = 0;  // 上次编码器值
float INS_Last_Yaw = 0.0f;   // 上一次航向角(rad)

void INS_Update(void)
{
    /* 读取累计编码器 */
    int32 Encoder_Now = (Read_Encoder_Aggregate_Value(TIM5_ENCODER) + Read_Encoder_Aggregate_Value(TIM2_ENCODER))/2;

    /* 计算编码器增量 */
    int32 delta_encoder = Encoder_Now - INS_Last_Encoder;

    /* 保存本次编码器值 */
    INS_Last_Encoder = Encoder_Now;

    /* 脉冲转距离 */
    float ds = (WHEEL_C / 2048.0f) * delta_encoder;

    /* 当前Yaw（映射值）：角度 → 弧度 */
    float Yaw_Now = Yaw_Mapped * PI / 180.0f;

    /*
     * 处理Yaw 0°/360°跳变
     * 例如：
     * 上一次 = 359°
     * 当前   = 1°
     * 不能直接平均成180°
     */
    float yaw_diff = Yaw_Now - INS_Last_Yaw;

    if (yaw_diff > PI)
    {
        yaw_diff -= 2.0f * PI;
    }
    else if (yaw_diff < -PI)
    {
        yaw_diff += 2.0f * PI;
    }

    /* 得到10ms期间的中间Yaw */
    float Yaw_Mid = INS_Last_Yaw + yaw_diff * 0.5f;

    /* 更新二维坐标 */
    INS_X += ds * sinf(Yaw_Mid);
    INS_Y += ds * cosf(Yaw_Mid);

    /* 保存当前Yaw，供下一次使用 */
    INS_Last_Yaw = Yaw_Now;

    /* 保存当前航向角 */
    INS_Yaw = Yaw_Now;
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
int32 INS_Point_Count=0;                // 当前路线已录点数
void INS_write(void)
{
    int32 delta2;
    int32 pack;

    Read_Continue_Encoder_Left=(Read_Encoder_Aggregate_Value(TIM5_ENCODER) + Read_Encoder_Aggregate_Value(TIM2_ENCODER))/2;
    delta2 = Read_Continue_Encoder_Left - Last_Encoder;

    if((abs(delta2) >= Nag_Set_mileage) && INS_Point_Count < NagPoint_MAX)
    {
        int16 x_int = (int16)(INS_X * 100.0f);
        int16 y_int = (int16)(INS_Y * 100.0f);

        pack = (((uint32)(uint16)x_int) << 16) | ((uint16)y_int);

        INS_data[INS_Point_Count++] = pack;
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
    // 根据当前线路索引选择对应页码（同一扇区，不同页）
    int32 page = FLASH_PAGE_BASE + current_route_index;

    flash_buffer_clear();

    // 存点数 + 坐标原点
    flash_union_buffer[500].uint32_type = INS_Point_Count;
    flash_union_buffer[501].float_type = INS_X;   // 录制时的X坐标
    flash_union_buffer[502].float_type = INS_Y;   // 录制时的Y坐标
    flash_union_buffer[503].int32_type = (int32)(Yaw_Mapped * 10000.0f);  // 录制时的映射Yaw（×10000）
    for(int i = 0; i < INS_Point_Count; i++)
    {
        flash_union_buffer[i].uint32_type = INS_data[i];
    }

    if(flash_check(FLASH_SECTOR, page))
    {
       flash_erase_page(FLASH_SECTOR, page);
    }

    flash_write_page_from_buffer(FLASH_SECTOR, page);

    ips200_show_string(0, 16*10, "INS flash");
}

// 【新增】进入修点时暂存的原始元数据，保存时原样写回
static int32  saved_Point_Count = 0;
static float  saved_INS_X = 0.0f;
static float  saved_INS_Y = 0.0f;
static int32  saved_Yaw_x10000 = 0;

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
int32 INS_Resd_Point_Count=0;           // 读取到的路线点数
void INS_FlashRead(void)
{
    // 根据当前线路索引选择对应页码（同一扇区，不同页）
    int32 page = FLASH_PAGE_BASE + current_route_index;

    if(flash_check(FLASH_SECTOR, page))
    {
        flash_read_page_to_buffer(FLASH_SECTOR, page);

        INS_Resd_Point_Count = flash_union_buffer[500].uint32_type;
        INS_X = flash_union_buffer[501].float_type;   // 恢复录制时的X坐标
        INS_Y = flash_union_buffer[502].float_type;   // 恢复录制时的Y坐标
        float flash_yaw = flash_union_buffer[503].int32_type / 10000.0f;  // 读取录制时的Yaw

        saved_Point_Count = flash_union_buffer[500].uint32_type;  // 暂存原始元数据
        saved_INS_X = flash_union_buffer[501].float_type;
        saved_INS_Y = flash_union_buffer[502].float_type;
        saved_Yaw_x10000 = flash_union_buffer[503].int32_type;


        Set_Map_Yaw(flash_yaw);  // 设置映射偏移，使Yaw_Mapped=录制时的Yaw
        for(int i = 0; i < INS_Resd_Point_Count; i++)
        {
            INS_data2[i] = flash_union_buffer[i].uint32_type;
        }
        for(int i = 0; i < INS_Resd_Point_Count / 2; i++)
        {
            int32 temp = INS_data2[i];
            INS_data2[i] = INS_data2[INS_Resd_Point_Count - 1 - i];
            INS_data2[INS_Resd_Point_Count - 1 - i] = temp;
        }

        ips200_show_string(172, 16*11, "OK flash");
    }
    else
    {
        ips200_show_string(172, 16*11, "NO flash");
    }
}
/******************************************方*******************************************************/
// ======================================================================
// 【新增】旋转编码器修点（逐点微调 INS_data2 的 X/Y 坐标）
//   操作：空闲态旋钮=选点；key1=空闲→修X→修Y→空闲；修X/Y态旋钮=±1cm；key2=存flash
// ======================================================================
typedef enum
{
    INS_EDIT_IDLE,   // 空闲：旋钮切换选中点
    INS_EDIT_X,      // 修 X 坐标
    INS_EDIT_Y,      // 修 Y 坐标
} INS_EditState_Enum;

INS_EditState_Enum INS_edit_state = INS_EDIT_IDLE;  // 当前修点状态
int32 INS_Edit_Point = 0;                           // 当前选中的点索引
uint8 INS_edit_flag  = 0;                           // 修点模式标志（进入时置1，离开复位）
uint8 Encoder_Btn_Short = 0;                        // 【新增】旋钮按键短按锁存（进入修点）
uint8 Encoder_Btn_Long  = 0;                        // 【新增】旋钮按键长按锁存（退出修点）
uint8 INS_point_mark[NagPoint_MAX] = {0};           // 【新增】标记点数组（key3标记/key4清除，整体修点用）



// 【新增】修点完成后，把 INS_data2 写回当前线路对应的 flash 页
void INS_FlashWrite_Edit(void)
{
    int32 page = FLASH_PAGE_BASE + current_route_index;

    flash_buffer_clear();

    flash_union_buffer[500].uint32_type = saved_Point_Count;  // 原样写回，不受INS_Update干扰
    flash_union_buffer[501].float_type = saved_INS_X;
    flash_union_buffer[502].float_type = saved_INS_Y;
    flash_union_buffer[503].int32_type = saved_Yaw_x10000;    // Yaw也原样写回

    // 【修复】不原地反转 INS_data2，直接倒序写入 Flash
    // INS_data2 在编辑时是反转序 [Pn-1, ..., P0]
    // Flash 需要原始序 [P0, ..., Pn-1]，所以倒序搬运即可
    for(int i = 0; i < saved_Point_Count; i++)
    {
        flash_union_buffer[i].uint32_type = INS_data2[saved_Point_Count - 1 - i];
    }

    if(flash_check(FLASH_SECTOR, page))
    {
       flash_erase_page(FLASH_SECTOR, page);
    }

    flash_write_page_from_buffer(FLASH_SECTOR, page);

    ips200_show_string(0, 16*10, "INS save");
}

// 【新增】旋转编码器修点（非阻塞状态机，由 System_INS 在修点拨码组合下反复调用）
// 【新增】修点路径图静态缓冲（避免大数组占用栈空间）
static int16 INS_path_sx[NagPoint_MAX];
static int16 INS_path_sy[NagPoint_MAX];

// 【新增】绘制当前 INS 路线的路径图（X/Y 平面坐标），并高亮选中点
void INS_Draw_Path(void)
{
    int i;
    int n = INS_Resd_Point_Count;

    if(n <= 1)
    {
        return;   // 点数不足，无法画图
    }

    // 1. 求所有点的 X/Y 范围（单位 m）
    float min_x = (float)(int16)(INS_data2[0] >> 16) / 100.0f;
    float max_x = min_x;
    float min_y = (float)(int16)(INS_data2[0] & 0xFFFF) / 100.0f;
    float max_y = min_y;

    for(i = 1; i < n; i++)
    {
        float xi = (float)(int16)(INS_data2[i] >> 16) / 100.0f;
        float yi = (float)(int16)(INS_data2[i] & 0xFFFF) / 100.0f;
        if(xi < min_x) min_x = xi;
        if(xi > max_x) max_x = xi;
        if(yi < min_y) min_y = yi;
        if(yi > max_y) max_y = yi;
    }

    float w = max_x - min_x;
    float h = max_y - min_y;
    if(w < 0.05f) w = 0.05f;
    if(h < 0.05f) h = 0.05f;

    // 2. 显示区域（屏幕中下部，避开顶部文字和底部提示）
    int16 rx = 8;
    int16 ry = 100;
    int16 rw = 224;
    int16 rh = 180;

    // 3. 等比缩放 + 居中
    float scale = (w / rw > h / rh) ? (rw / w) : (rh / h);
    float ox = (rw - w * scale) / 2.0f;
    float oy = (rh - h * scale) / 2.0f;

    // 4. 计算各点屏幕坐标
    for(i = 0; i < n; i++)
    {
        float xi = (float)(int16)(INS_data2[i] >> 16) / 100.0f;
        float yi = (float)(int16)(INS_data2[i] & 0xFFFF) / 100.0f;
        INS_path_sx[i] = rx + (int16)((xi - min_x) * scale + ox);
        INS_path_sy[i] = ry + (int16)(rh - (yi - min_y) * scale - oy);  // 翻转Y
    }

    // 5. 画路径连线
    for(i = 0; i < n - 1; i++)
    {
        ips200_draw_line(INS_path_sx[i], INS_path_sy[i],
                         INS_path_sx[i+1], INS_path_sy[i+1], RGB565_WHITE);
    }

    // 6. 画点：普通点小圆，选中点黄色双圈高亮，标记点青色
    for(i = 0; i < n; i++)
    {
        if(i == INS_Edit_Point)
        {
            ips200_draw_circle(INS_path_sx[i], INS_path_sy[i], 5, RGB565_YELLOW);
            ips200_draw_circle(INS_path_sx[i], INS_path_sy[i], 6, RGB565_YELLOW);
            if(INS_point_mark[i])  // 选中点同时被标记，再加青色外圈
            {
                ips200_draw_circle(INS_path_sx[i], INS_path_sy[i], 8, RGB565_CYAN);
            }
        }
        else if(INS_point_mark[i])
        {
            ips200_draw_circle(INS_path_sx[i], INS_path_sy[i], 4, RGB565_CYAN);  // 【新增】标记点
        }
        else
        {
            ips200_draw_circle(INS_path_sx[i], INS_path_sy[i], 2, RGB565_GRAY);
        }
    }
}

void INS_Point_Edit(void)
{
    int enc;
    int32 pack;
    int16 x, y;
    int i;

    // 旋钮采样（每帧一次，累计步数）
    menu_encoder_update();

    // ---- key1：循环切换 空闲 → 修X → 修Y → 空闲 ----
    if(key1_flag)
    {
        // ---- 全屏重绘（修点模式下车是停着的，重绘开销可接受）----
        ips200_clear();
        key1_flag = 0;
        switch(INS_edit_state)
        {
            case INS_EDIT_IDLE: INS_edit_state = INS_EDIT_X;    break;
            case INS_EDIT_X:    INS_edit_state = INS_EDIT_Y;    break;
            case INS_EDIT_Y:    INS_edit_state = INS_EDIT_IDLE; break;
            default:            INS_edit_state = INS_EDIT_IDLE; break;
        }
        Buzzer_check(30);
    }

    // ---- key2：保存写回当前线路 flash ----
    if(key2_flag)
    {
        // ---- 全屏重绘（修点模式下车是停着的，重绘开销可接受）----
        ips200_clear();
        key2_flag = 0;
        INS_FlashWrite_Edit();
        Buzzer_check(50);
    }

    // ---- key3：标记/取消标记当前点（整体修点用）----
    if(key3_flag)
    {
        ips200_clear();
        key3_flag = 0;
        if(INS_Resd_Point_Count > 0)
        {
            INS_point_mark[INS_Edit_Point] = !INS_point_mark[INS_Edit_Point];  // 切换标记
        }
        Buzzer_check(30);
    }

    // ---- key4：清除所有标记点 ----
    if(key4_flag)
    {
        ips200_clear();
        key4_flag = 0;
        for(i = 0; i < INS_Resd_Point_Count; i++)
        {
            INS_point_mark[i] = 0;
        }
        Buzzer_check(30);
    }

    // ---- 旋钮 ----
    enc = menu_encoder_get_change();
    if(enc != 0)
    {
        ips200_clear();   // 【新增】有改动就清屏，避免路径图残影
        if(INS_Resd_Point_Count <= 0)
        {
            // 没有路线数据，忽略旋钮
        }
        else if(INS_edit_state == INS_EDIT_IDLE)
        {
            // 空闲态：切换选中点，循环
            INS_Edit_Point += enc;
            while(INS_Edit_Point >= INS_Resd_Point_Count) INS_Edit_Point -= INS_Resd_Point_Count;
            while(INS_Edit_Point < 0) INS_Edit_Point += INS_Resd_Point_Count;
        }
        else
        {
            // 判断是否有标记点：有 → 整体移动所有标记点；无 → 只移当前点
            uint8 has_mark = 0;
            for(i = 0; i < INS_Resd_Point_Count; i++)
            {
                if(INS_point_mark[i]) { has_mark = 1; break; }
            }

            if(has_mark)
            {
                // 【新增】整体修点：所有标记点一起 ±1cm
                for(i = 0; i < INS_Resd_Point_Count; i++)
                {
                    if(INS_point_mark[i])
                    {
                        pack = INS_data2[i];
                        x = (int16)(pack >> 16);
                        y = (int16)(pack & 0xFFFF);
                        if(INS_edit_state == INS_EDIT_X) x += (int16)enc * INS_EDIT_STEP;
                        else                              y += (int16)enc * INS_EDIT_STEP;
                        INS_data2[i] = (((uint32)(uint16)x) << 16) | ((uint16)y);
                    }
                }
            }
            else
            {
                // 单点修：只移当前点
                pack = INS_data2[INS_Edit_Point];
                x = (int16)(pack >> 16);
                y = (int16)(pack & 0xFFFF);
                if(INS_edit_state == INS_EDIT_X) x += (int16)enc * INS_EDIT_STEP;
                else                              y += (int16)enc * INS_EDIT_STEP;
                INS_data2[INS_Edit_Point] = (((uint32)(uint16)x) << 16) | ((uint16)y);
            }
        }
    }

    // 路径图（含选中点高亮）
    INS_Draw_Path();

    // ---- 文字信息 ----
    pack = (INS_Resd_Point_Count > 0) ? INS_data2[INS_Edit_Point] : 0;
    x = (int16)(pack >> 16);
    y = (int16)(pack & 0xFFFF);

    //ips200_show_string(0, 16*0, "Route:");  ips200_show_int(60, 16*0, current_route_index + 1, 1);
    ips200_show_string(0, 16*1, "Edit");    // 修点状态标签
    ips200_show_string(0, 16*2, "N:");      ips200_show_int(30, 16*2, INS_Edit_Point, 3);
    ips200_show_string(0, 16*3, "X:");      ips200_show_int(30, 16*3, x, 6);   // 当前点 X（cm）
    ips200_show_string(0, 16*4, "Y:");      ips200_show_int(30, 16*4, y, 6);   // 当前点 Y（cm）

    if(INS_edit_state == INS_EDIT_X)      ips200_show_string(120, 16*1, "X");
    else if(INS_edit_state == INS_EDIT_Y) ips200_show_string(120, 16*1, "Y");
    else                                  ips200_show_string(120, 16*1, "-");

    ips200_show_string(0, 16*18, "K1:mode K2:save K3:mark K4:clr");
}
/****************************************方*******************************************************************/

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


int32 total = 0;  // 总点数
/************************************************
 *  函数名称: INS_OneKeyReturn
 *  功能说明：一键原路返回，不经过Flash，直接从内存取数据
 *           1. 将录制数据从 INS_data 拷贝到 INS_data2
 *           2. 反转坐标路径（倒序 + 坐标变换到当前位置为原点）
 *           3. 重置INS原点为当前位置
 *           4. 启动路径复现
 *  使用方法：倒车录制结束后，按一个键，车自动原路返回
 ************************************************/
void INS_OneKeyReturn(void)
{
    // 1. 先根据当前线路索引从Flash读取对应路线数据
    INS_FlashRead();

    if(INS_Resd_Point_Count == 0) return;

    // 2. 停止录制
    INS_Flag = 0;
    // 4. 重置复现状态
    next = 0;

    // 5. 启动路径复现
    dispatch_car = 1;
    Camera_Navigation_Motor_flag = 1;
    Back_Motor_Set_Speed = Motor_Speed_GS;
    Back_Motor_SetPwm(Motor_Speed_GS, Motor_Speed_GS);
    INS_Flag = 3;
}


float INS_two_points_distance = 0;


/************************************************
 *  函数名称: INS_track_oneway
 *  功能说明：单向路径复现（只有前进，无后退段）
 *           配合 INS_OneKeyReturn 使用
 ************************************************/
void INS_track_oneway(void)
{
    int16 total = (int16)INS_Resd_Point_Count;

    int32 pack = INS_data2[next];
    int16 x_int = (int16)(pack >> 16);
    int16 y_int = (int16)(pack & 0xFFFF);
    float x = x_int / 100.0f;
    float y = y_int / 100.0f;
    
    // 最后一个点Y坐标偏移
    if(next >= total - 7)
    {
        y = (y_int / 100.0f) + (LAST_POINT_Y_OFFSET_CM * 0.01f);
        x = (x_int / 100.0f) + (LAST_POINT_X_OFFSET_CM * 0.01f);
    }
    
    float INS_Error = 0;
    float INS_azimuth = 0;

    ips200_show_int( 80, 16*2, total, 3);

    INS_two_points_distance = Get_Two_Point_Distance(INS_X, INS_Y, x, y);
    INS_azimuth = Get_Target_Angle(INS_X, INS_Y, x, y);

    ips200_show_int(0, 100, next, 2);

    ips200_show_string(0, 16*3, "x:");
    ips200_show_string(0, 16*4, "Y:");
    ips200_show_float(60, 16*3, x, 3, 1);
    ips200_show_float(60, 16*4, y, 3, 1);

    ips200_show_string(0, 16*5, "INS_X:");
    ips200_show_string(0, 16*6, "INS_Y:");
    ips200_show_float(60, 16*5, INS_X, 3, 1);
    ips200_show_float(60, 16*6, INS_Y, 3, 1);

    // 距离目标点足够近 → 前进到下一个点
    if(INS_two_points_distance < INS_distance && next < total - 1)
    {
        if(next == 0)
        {
            Back_Motor_PID_Start_flag = 1;
        }
        next++;
        Back_Motor_Set_Speed = Motor_Speed_GS;
        Buzzer_SetTimer(50);
    }

    // 电机输出
    if(next >= 1 && next < total && Camera_Navigation_Motor_flag == 1)
    {
        Back_Motor_SetPwm(Back_Motor_Set_Speed, Back_Motor_Set_Speed);
//        ips200_show_int(10*16 , 16*11, 1, 2);
    }

    // 方向PID
    if(dispatch_car == 1 && next < total)
    {
        INS_Error = -angle_diff(INS_azimuth,((Yaw_Mapped+180) >=360 ) ? (Yaw_Mapped-180) : (Yaw_Mapped+180));
        Angle_Target_Position = -GPS_IMU_PID_Start(Open, INS_Error, 0);
    }

    // 到达终点 → 停车
    if(INS_two_points_distance < INS_distance_final && next >= total - 1)
    {
        next = total;
        dispatch_car = 0;
        Back_Motor_Set_Speed = 0;
        Back_Motor_SetPwm(0, 0);
        Back_Motor_PID_Start_flag = 0;



        Camera_Navigation_Motor_flag = 1;
        dispatch_car = 1;
        INS_Flag = 4;
        system_delay_ms(2000);
        INS_MH_Return();
        Buzzer_SetTimer(1000);
    }
}


void INS_track(void)
{

    int32 pack = INS_data2[next];
    int16 x_int = (int16)(pack >> 16);
    int16 y_int = (int16)(pack & 0xFFFF);
    float x = x_int / 100.0f;
    float y = y_int / 100.0f;
    float INS_Error=0;
    float INS_azimuth = 0;

    INS_two_points_distance = Get_Two_Point_Distance(INS_X,INS_Y,x,y); //获取俩点间距离
    INS_azimuth = Get_Target_Angle(INS_X,INS_Y,x,y);    //获取俩点之间的角度

    ips200_show_int(0, 100, next, 2);//显示实时到达点位

    ips200_show_string(0, 16*3, "x:");
    ips200_show_string(0, 16*4, "Y:");
    ips200_show_float( 60, 16*3,x, 3,1);
    ips200_show_float( 60, 16*4,y, 3,1);

    ips200_show_string(0, 16*5, "INS_X:");
    ips200_show_string(0, 16*6, "INS_Y:");
    ips200_show_float( 60, 16*5,INS_X, 3,1);
    ips200_show_float( 60, 16*6,INS_Y, 3,1);

    if(INS_two_points_distance < INS_distance  && next==0)
    {
       next++;

       Back_Motor_PID_Start_flag = 1; //为1时开启位置PID
       Back_Motor_Set_Speed = 300;//设置速度
       Buzzer_SetTimer(50);
    }
    if(INS_two_points_distance < INS_distance &&  next!=0 &&  next < INS_Resd_Point_Count - 1)
    {
       next++;
       Back_Motor_Set_Speed = 300;//设置速度
       Buzzer_SetTimer(50);
    }

      //电机速度模式选择
    if(next >= 1 && next < INS_Resd_Point_Count)
    {
      Back_Motor_SetPwm(Back_Motor_Set_Speed,Back_Motor_Set_Speed);      //电机速度赋值
    }


      //方向PID选择是否开启
    if(dispatch_car == 1)
    {
        INS_Error = angle_diff(INS_azimuth,Yaw_Mapped);    // 方向角与航向角之间的误差（用于控制）
        Angle_Target_Position = -GPS_IMU_PID_Start(Open,INS_Error,0);
    }

    if(INS_two_points_distance < INS_distance_final  && next >= INS_Resd_Point_Count - 1)
    {
        next = (int16)INS_Resd_Point_Count;
        delay_ScanTemp = 0;//关闭定时器延迟发车标志位
        dispatch_car = 0;//关闭发车标志位
        Back_Motor_Set_Speed = 0;//设置速度
        Back_Motor_SetPwm(0,0);
        Back_Motor_PID_Start_flag = 0; //关闭位置PID，给电机速度为0
        INS_Flag = 0;
        Buzzer_SetTimer(1000);
    }

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
/************************************************
 *  函数名称: 前进到目标点再返回原点
 *  功能说明: 先前进到 (INS_X_MH, INS_Y_MH)，
 *           到达后再返回 (0,0)
 *  使用方法: 设定 INS_X_MH / INS_Y_MH，
 *           INS_Flag = 4 启动
 *  修改时间: 2026年8月21日
 ************************************************/
// 【非阻塞延时】阶段切换等待计数器（配合10ms周期调用）
static int32 mh_delay_count = 0;
#define MH_TURN_DELAY   80   // 换向等待：80 × 10ms = 800ms

void INS_MH_Return(void)
{
    float target_x, target_y;
    float distance, azimuth, INS_Error;

    // ===== 首次进入：初始化所有必要标志位 =====
    if(INS_mh_phase == 0 && Back_Motor_PID_Start_flag == 0)
    {
        dispatch_car = 1;
        Camera_Navigation_Motor_flag = 1;
        Back_Motor_PID_Start_flag = 1;
        Back_Motor_Set_Speed = 200;
        mh_delay_count = 0;
    }

    // ===== 换向等待（非阻塞） =====
    if(mh_delay_count > 0)
    {
        Back_Motor_SetPwm(0, 0);   // 等待期间保持停车
        mh_delay_count--;
        return;                     // 等待结束后再继续导航
    }

    // 根据当前阶段确定目标点
    if(INS_mh_phase == 0)
    {
        target_x = INS_X_MH +(LAST_POINT_X_OFFSET_CM / 100);
        target_y = INS_Y_MH;
    }
    else
    {
        target_x = 0.0f +(LAST_POINT_X_OFFSET_CM / 100);
        target_y = 0.0f;
    }

    // 计算距离与方位角
    distance = Get_Two_Point_Distance(INS_X, INS_Y, target_x, target_y);
    azimuth  = Get_Target_Angle(INS_X, INS_Y, target_x, target_y);

    // 显示信息
    ips200_show_string(0, 16*2, "MH Ph:");  ips200_show_int(70, 16*2, INS_mh_phase, 1);
    ips200_show_string(0, 16*3, "Dist:");   ips200_show_float(70, 16*3, distance, 4, 2);
    ips200_show_string(0, 16*4, "TX:");     ips200_show_float(70, 16*4, target_x, 3, 1);
    ips200_show_string(0, 16*5, "TY:");     ips200_show_float(70, 16*5, target_y, 3, 1);
    ips200_show_string(0, 16*6, "INS_X:");  ips200_show_float(70, 16*6, INS_X, 3, 1);
    ips200_show_string(0, 16*7, "INS_Y:");  ips200_show_float(70, 16*7, INS_Y, 3, 1);

    // 到达判断

        if(INS_mh_phase == 0 && distance < 0.3f)
        {
            // 到达前进目标点 → 非阻塞等待后切换返回
            INS_mh_phase = 1;
            Back_Motor_Set_Speed = -200;
            mh_delay_count = MH_TURN_DELAY;
            Buzzer_SetTimer(200);
            return;
        }
        else if(INS_mh_phase == 1 && distance < 0.5f)
        {
            // 到达原点(0,0) → 停车
            dispatch_car = 0;
            Back_Motor_SetPwm(0, 0);
            Back_Motor_PID_Start_flag = 0;
            Camera_Navigation_Motor_flag = 1;
            INS_Flag = 0;
            Buzzer_SetTimer(1000);
            return;
        }


    // 方向PID控制
    if(dispatch_car == 1)
    {
        if(INS_mh_phase == 0)
        {
            // 前进阶段：标准正向PID
            INS_Error = angle_diff(azimuth, Yaw_Mapped);
            Angle_Target_Position = -GPS_IMU_PID_Start(Open, INS_Error, 0);
        }
        else
        {
            // 返回阶段：180°反转PID（车头朝反方向后退）
            INS_Error = -angle_diff(azimuth,((Yaw_Mapped+180) >=360 ) ? (Yaw_Mapped-180) : (Yaw_Mapped+180));
            Angle_Target_Position = -GPS_IMU_PID_Start(Open, INS_Error, 0);
        }
    }

    // 电机驱动
    if(Camera_Navigation_Motor_flag == 1)
    {
        Back_Motor_SetPwm(Back_Motor_Set_Speed, Back_Motor_Set_Speed);
    }
}






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
            INS_track_oneway();
            break;
        case 4:
            INS_MH_Return();
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
    // 显示当前线路编号
    ips200_show_string(0, 16*9, "Route:");
    ips200_show_int(60, 16*9, current_route_index + 1, 1);
/*****************************方***************************************/
    // 【修改】旋钮按键（P11_11）：短按进入修点，长按退出（任何拨码模式都有效）
    if(Encoder_Btn_Short)
    {
        Encoder_Btn_Short = 0;
        if(!INS_edit_flag)
        {
            INS_edit_flag = 1;
            INS_FlashRead();                // 读取当前线路到 INS_data2

            INS_Edit_Point = 0;             // 选中点归 0
            INS_edit_state = INS_EDIT_IDLE; // 状态回到空闲
            menu_encoder_init();            // 初始化旋钮
            memset(INS_point_mark, 0, sizeof(INS_point_mark));  // 【新增】进入修点清空标记
        }
    }
    if(Encoder_Btn_Long)
    {
        Encoder_Btn_Long = 0;
        INS_edit_flag = 0;                  // 只退出，不保存
        ips200_clear();                     // 【新增】退出修点后清屏，去掉路径图残留
    }

    // 修点模式下独占运行，跳过下面的正常分支
    if(INS_edit_flag)
    {
        INS_Point_Edit();
        return;
    }
/*****************************方***************************************/
    if(!gpio_get_level(sw1) &&  !gpio_get_level(sw2))//1,2上
    {
        g_debug_mode=0;
        ips200_show_string(230, 16*8, "S");
        if(key1_flag == 1 && INS_Flag == 0)
        {
            key1_flag=0;
            INS_Flag=1;
            Buzzer_check(50);
        }
//        if(key2_flag == 1)
//        {
//            key2_flag=0;
//
//            Buzzer_check(50);
//        }
         if(key3_flag == 1)
         {
             key3_flag=0;
             INS_Flag=0;
             INS_FlashWrite();
             INS_OneKeyReturn();
             Buzzer_check(50);
         }
        if(key4_flag == 1)
        {
            key4_flag=0;
            INS_OneKeyReturn();   //一键返回
            Buzzer_check(50);
        }
        // if(key5_flag == 1)
        // {
        //     key5_flag = 0;
        //     Buzzer_check(50);
        // }
    }
    else if(gpio_get_level(sw1) &&  gpio_get_level(sw2))//1,2下
    {
        g_debug_mode=0;
        ips200_show_string(230, 16*8, "X");
        if(key1_flag == 1)
        {
            key1_flag=0;
            INS_FlashWrite();   //（存flash）
            INS_Flag=0;
            Buzzer_check(50);
        }
        if(key2_flag == 1)
        {
            key2_flag=0;
            Buzzer_check(50);
            INS_FlashRead();   //读取当前线路flash
            INS_Flag=0;
        }
        if(key3_flag == 1)
        {
            key3_flag=0;
            Buzzer_check(50);
            // 3. 倒序数据（原路返回）


            // 4. 重置复现状态
            next = 0;

            // 5. 启动路径复现
            dispatch_car = 1;
            Back_Motor_Set_Speed = Motor_Speed_GS;
            Back_Motor_SetPwm(Motor_Speed_GS, Motor_Speed_GS);
            INS_Flag = 3;
        }
        if(key4_flag == 1)//切换线路
        {
            key4_flag=0;
            current_route_index++;
            if(current_route_index >= MAX_ROUTES)
            {
                current_route_index = 0;
            }
            Buzzer_check(50);
        }

    }
    else if(gpio_get_level(sw1) &&  !gpio_get_level(sw2))//1下,2上
    {
        g_debug_mode=1;
    }
    else if(!gpio_get_level(sw1) &&  gpio_get_level(sw2))//1shang,2下
    {
        g_debug_mode=0;
        if(key3_flag == 1)
        {
            key3_flag=0;
            Camera_Navigation_Motor_flag = 1;
            dispatch_car = 1;
            INS_Flag = 4;
            Buzzer_SetTimer(50);
        }
//                if(key3_flag == 1)
//                {
//                    Buzzer_check(50);
//                    key3_flag=0;
//                    Key8_Num=1;
//                }
//                if(key4_flag == 1)
//                {
//                    Buzzer_check(50);
//                    key4_flag=0;
//                    Key8_Num=0;
//                }
    }
}
