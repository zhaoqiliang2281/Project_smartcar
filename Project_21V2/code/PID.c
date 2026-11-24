/*
 * PID.c
 *
 *  Created on: 2026年3月15日
 *      Author: LiangZhaoqi
 */

#include "zf_common_headfile.h"

#define LimitMax(input, max)   \
{                              \
        if (input > max)       \
        {                      \
            input = max;       \
        }                      \
        else if (input < -max) \
        {                      \
            input = -max;      \
        }                      \
}

#define LimitMan(input, man)                        \
{                                                   \
        if ((input) > -(man) && (input) < (man))    \
        {                                           \
            (input) = 0.0;                          \
        }                                           \
}
/************************************************
 * 函数名称: PID初始化
 * 功能说明：初始化PID控制器的参数和状态
 * 参数说明：
 *   pid: PID结构体指针，指向要初始化的PID对象
 *   mode: PID工作模式，PID_POSITION(位置式) 或 PID_DELTA(增量式)
 *   PID: 包含三个PID参数的数组，[0]=Kp, [1]=Ki, [2]=Kd
 *   max_out: 最大输出限制值
 *   max_iout: 最大积分输出限制值
 * 函数返回：无
 * 修改时间：2025年6月16日
 * 备   注：必须先调用此函数初始化PID控制器
 ************************************************/
void PID_init(pid_type_def *pid, uint8_t mode, const fp32 PID[3], fp32 max_out, fp32 max_iout)
{
    if (pid == NULL || PID == NULL)
    {
        return;
    }

    pid->mode = mode;

    pid->Kp = PID[0];
    pid->Ki = PID[1];
    pid->Kd = PID[2];

    pid->max_out = max_out;
    pid->max_iout = max_iout;

    pid->Dbuf[0] = pid->Dbuf[1] = pid->Dbuf[2] = 0.0f;

    pid->error[0] = pid->error[1] = pid->error[2] = 0.0f;
    pid->Pout = pid->Iout = pid->Dout = pid->out = 0.0f;
}

/************************************************
 * 函数名称: PID计算
 * 功能说明：执行PID算法计算，根据设定值和反馈值计算控制输出
 * 参数说明：
 *   pid: PID结构体指针，指向已初始化的PID对象
 *   ref: 反馈值，系统当前的实际测量值
 *   set: 设定值，期望达到的目标值
 * 函数返回：PID控制器的输出值
 * 修改时间：2025年6月16日
 * 备   注：这是PID控制的核心函数，需周期性调用
 ************************************************/
fp32 PID_Update(pid_type_def *pid, fp32 ref, fp32 set)
{
    if (pid == NULL)
    {
        return 0.0f;
    }

    pid->error[2] = pid->error[1];
    pid->error[1] = pid->error[0];

    pid->set = set;
    pid->fdb = ref;

    pid->error[0] = pid->set - pid->fdb;

    if (pid->mode == PID_POSITION)
    {
        pid->Pout = pid->Kp * pid->error[0];

        pid->Iout += pid->Ki * pid->error[0];

        pid->Dbuf[2] = pid->Dbuf[1];
        pid->Dbuf[1] = pid->Dbuf[0];

        pid->Dbuf[0] = (pid->error[0] - pid->error[1]);

        pid->Dout = pid->Kd * pid->Dbuf[0];

        LimitMax(pid->Iout, pid->max_iout);

        pid->out = pid->Pout + pid->Iout + pid->Dout;

        LimitMax(pid->out, pid->max_out);
    }
    else if (pid->mode == PID_DELTA)
    {

        pid->Pout = pid->Kp * (pid->error[0] - pid->error[1]);

        pid->Iout = pid->Ki * pid->error[0];

        pid->Dbuf[2] = pid->Dbuf[1];
        pid->Dbuf[1] = pid->Dbuf[0];

        pid->Dbuf[0] = (pid->error[0] - 2.0f * pid->error[1] + pid->error[2]);

        pid->Dout = pid->Kd * pid->Dbuf[0];

        pid->out += pid->Pout + pid->Iout + pid->Dout;

        LimitMan(pid->out, 0.1f);
        LimitMax(pid->out, pid->max_out);
    }
    else if(pid->mode == PID_LOCATION)
    {
        pid->Iout += pid->error[0];

        if(pid->Iout >   pid->max_iout) pid->Iout  = pid->max_iout;
        if(pid->Iout < -(pid->max_iout)) pid->Iout = -pid->max_iout;

        pid->out = (pid->Kp * pid->error[0])
                 + (pid->Ki * pid->Iout)
                 + (pid->Kd * (pid->error[0] - pid->error[1]));

        LimitMax(pid->out, pid->max_out);
    }
    return pid->out;
}

/************************************************
 * 函数名称: PID清除
 * 功能说明：清空PID控制器的所有状态变量，用于复位或重新开始控制
 * 参数说明：
 *   pid: PID结构体指针，指向要清除的PID对象
 * 函数返回：无
 * 修改时间：2025年6月16日
 * 备   注：在切换控制目标或重新开始控制时调用
 ************************************************/
void PID_clear(pid_type_def *pid)
{
    if (pid == NULL)
    {
        return;
    }
    pid->error[0] = pid->error[1] = pid->error[2] = 0.0f;

    pid->Dbuf[0] = pid->Dbuf[1] = pid->Dbuf[2] = 0.0f;

    pid->out = pid->Pout = pid->Iout = pid->Dout = 0.0f;

    pid->fdb = pid->set = 0.0f;
}

/************************************************
 * 函数名称: PID参数设置
 * 功能说明：
 * 参数说明：pid：PID结构体指针，指向要清除的PID对象
 *         Kp：比例系数
 *         Ki：积分系数
 *         Kd：微分系数
 * 函数返回：无
 * 修改时间：2026年4月25日
 * 备   注：需要动态调参时使用，主循环不断调用就可实现动态调参
 ************************************************/
void PID_Parameter_Set(pid_type_def *pid, float Kp,float Ki,float Kd)
{
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
}

pid_type_def PID_GPS_IMU;   //GPS与IMUPID
pid_type_def PID_Diversion; //转向PID
pid_type_def PID_Location;  //位置PID
pid_type_def PID_Angle;     //角度PID

/************************************************
 *  函数名称: 全部PID初始化
 *  功能说明：速度与位置PID串级
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年3月15日
 *  备   注：初始化使用，只调用一次
 ************************************************/
void All_PID_Init(void)
{
    //GPS导航 PID
    float GPS_IMU_PID_Value[3]={GPS_IMU_PID_KP,GPS_IMU_PID_KI,GPS_IMU_PID_KD};
    PID_init(&PID_GPS_IMU,PID_LOCATION,GPS_IMU_PID_Value,900,100);

    //转向PID
    float Diversion_PID_Value[3]={Diversion_PID_KP,Diversion_PID_KI,Diversion_PID_KD};
    PID_init(&PID_Diversion,PID_POSITION,Diversion_PID_Value,800,200);

    //位置PID
    float Location_PID_Value[3]={Llocation_PID_KP,Llocation_PID_KI,Llocation_PID_KD};
    PID_init(&PID_Location,PID_POSITION,Location_PID_Value,300,100);

    //角度PID
    float Angle_PID_Value[3]={Angle_PID_KP,Angle_PID_KI,Angle_PID_KD};
    PID_init(&PID_Angle,PID_POSITION,Angle_PID_Value,300,100);
}

/************************************************
 *  函数名称: PID更新参数
 *  功能说明：
 *  参数说明：
 *  参   数：PID结构体指针
 *  函数返回：无
 *  修改时间：2026年4月22日
 *  备   注：
 ************************************************/
void PID_Controller_Update_Parameter(pid_type_def *pid)
{
    if(pid->Start == Open)
    {
        pid->Result = PID_Update(pid,(float)pid->Actual,(float)pid->Target);
    }
    else
    {
        PID_clear(pid);
    }
}

/************************************************
 *  函数名称: PID计算
 *  功能说明：目前所有PID计算放在这里
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年4月22日
 *  备   注：放在中断调用：时间间隔：10ms
 ************************************************/
void PID_Controller_Update(void)
{
    static int8 Timer = 0;

    /*=========================PID 10ms 调用================================*/

    //PID速度内环
    PID_Controller_Update_Parameter(&PID_Right_Speed_Encoder);
    PID_Controller_Update_Parameter(&PID_Left_Speed_Encoder);

    /*=========================PID 20ms 调用================================*/
    Timer ++;
    if(Timer == 2)
    {
        Timer = 0;
        Angle_Encoder_PID_Update();    /*前轮角度传感器底层转向PID，勿动*/

        //读取编码器
        Read_Encoder_Value_Update(TIM2_ENCODER);
        Read_Encoder_Value_Update(TIM5_ENCODER);

        //GPS与IMU控制方向
        PID_Controller_Update_Parameter(&PID_GPS_IMU);

        PID_Controller_Update_Parameter(&PID_Diversion);    //转向PID
        PID_Controller_Update_Parameter(&PID_Location);     //位置PID
        PID_Controller_Update_Parameter(&PID_Angle);        //角度PID
    }
}

/************************************************
 *  函数名称：电机PID并传入参数
 *  功能说明：
 *  参数说明：范围：+-180
 *  参   数： Start：Open 表示开启PID，Close 关闭PID
 *          Left_Target： 左电机PID目标值
 *          Right_Target：右电机PID目标值
 *  函数返回：无
 *  修改时间：2026年4月22日
 *  备   注：
 ************************************************/
 void Motor_Speed_Encoder_PID_Start(PID_Start_enum Start,int32 Left_Target,int32 Right_Target)
 {
     PID_Left_Speed_Encoder.Start  = Start;
     PID_Right_Speed_Encoder.Start = Start;

     int16 Right_Encoder_Value = Unit_Time_Read_Encoder_Value(TIM2_ENCODER);    //右
     int16 Left_Encoder_Value  = Unit_Time_Read_Encoder_Value(TIM5_ENCODER);    //左

     PID_Left_Speed_Encoder.Actual  = (float)Left_Encoder_Value;
     PID_Left_Speed_Encoder.Target  = (float)Left_Target;

     PID_Right_Speed_Encoder.Actual = (float)Right_Encoder_Value;
     PID_Right_Speed_Encoder.Target = (float)Right_Target;
 }

 /************************************************
  *  函数名称: GPS与IMU开启PID并传入参数
  *  功能说明：
  *  参数说明：
  *  参   数： Start：Open 表示开启PID，Close 关闭PID
  *          Actual：PID实际值
  *          Target：PID目标值
  *  函数返回：PID计算的结果
  *  修改时间：2026年4月28日
  *  备   注：以下几个都是开启对应的PID
  ************************************************/
 int32 GPS_IMU_PID_Start(PID_Start_enum Start,int32 Actual,int32 Target)
 {
     PID_GPS_IMU.Start  = Start;
     PID_GPS_IMU.Actual = Actual;
     PID_GPS_IMU.Target = Target;

     return PID_GPS_IMU.Result;
 }

 /************************************************
  *  函数名称: 方向开启PID并传入参数
  *  功能说明：
  *  参数说明：
  *  参   数： Start：Open 表示开启PID，Close 关闭PID
  *          Actual：PID实际值
  *          Target：PID目标值
  *  函数返回：PID计算的结果
  *  修改时间：2026年4月28日
  *  备   注：以下几个都是开启对应的PID
  ************************************************/
 int32 Diversion_PID_Start(PID_Start_enum Start,int32 Actual,int32 Target)
 {
     PID_Diversion.Start  = Start;
     PID_Diversion.Actual = Actual;
     PID_Diversion.Target = Target;

     return PID_Diversion.Result;
 }

 /************************************************
  *  函数名称: 位置开启PID并传入参数
  *  功能说明：
  *  参数说明：
  *  参   数： Start：Open 表示开启PID，Close 关闭PID
  *          Actual：PID实际值
  *          Target：PID目标值
  *  函数返回：PID计算的结果
  *  修改时间：2026年4月28日
  *  备   注：以下几个都是开启对应的PID
  ************************************************/
 int32 Location_PID_Start(PID_Start_enum Start,int32 Actual,int32 Target)
{
    PID_Location.Start  = Start;
    PID_Location.Actual = Actual;
    PID_Location.Target = Target;

    return PID_Location.Result;
}

 /************************************************
  *  函数名称: 角度开启PID并传入参数
  *  功能说明：
  *  参数说明：
  *  参   数： Start：Open 表示开启PID，Close 关闭PID
  *          Actual：PID实际值
  *          Target：PID目标值
  *  函数返回：PID计算的结果
  *  修改时间：2026年4月28日
  *  备   注：以下几个都是开启对应的PID
  ************************************************/
 int32 Angle_PID_Start(PID_Start_enum Start,int32 Actual,int32 Target)
{
    PID_Angle.Start  = Start;
    PID_Angle.Actual = Actual;
    PID_Angle.Target = Target;

    return PID_Angle.Result;
}


