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
 * 函数说明: 初始化PID控制器的参数和状态
 * 输入说明:
 *   pid: PID结构体指针，指向要初始化的PID控制器
 *   mode: PID控制模式：PID_POSITION(位置式) 或 PID_DELTA(增量式)
 *   PID: 包含三个PID参数的数组指针，[0]=Kp, [1]=Ki, [2]=Kd
 *   max_out: 输出最大值限制
 *   max_iout: 积分项最大值限制
 * 输出说明:
 * 返回值:
 * 更新时间: 2025年6月16日
 * 备   注: 需要在调用前确保PID结构体已经声明
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
 * 函数名称: PID更新
 * 函数说明: 执行PID算法运算，根据给定值和反馈值计算输出控制量
 * 输入说明:
 *   pid: PID结构体指针，指向已经初始化的PID控制器
 *   ref: 反馈值，系统当前的实时测量值
 *   set: 给定值，期望达到的目标值
 * 输出说明: PID控制器的输出值
 * 返回值:
 * 更新时间: 2025年6月16日
 * 备   注: 根据不同的PID模式执行不同的计算方式
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
 * 函数说明: 清除PID控制器的所有历史数据和状态，恢复初始状态
 * 输入说明:
 *   pid: PID结构体指针，指向要清除的PID控制器
 * 输出说明:
 * 返回值:
 * 更新时间: 2025年6月16日
 * 备   注: 切换到新的控制目标时需要调用此函数
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
 * 函数说明:
 * 输入说明: pid为PID结构体指针，指向要设置的PID控制器
 *         Kp为比例系数
 *         Ki为积分系数
 *         Kd为微分系数
 * 输出说明:
 * 返回值:
 * 更新时间: 2026年4月25日
 * 备   注: 需要动态调整时使用，可以实时改变PID参数
 ************************************************/
void PID_Parameter_Set(pid_type_def *pid, float Kp,float Ki,float Kd)
{
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
}

pid_type_def PID_GPS_IMU;   //GPS和IMU PID
pid_type_def PID_Diversion; //转向PID
pid_type_def PID_Location;  //位置PID
pid_type_def PID_Angle;     //角度PID

/************************************************
 *  函数名称: 全部PID初始化
 *  函数说明: 初始化所有PID控制器
 *  输入说明:
 *  输   出:
 *  返回值:
 *  更新时间: 2026年3月15日
 *  备   注: 初始化使用，只需要调用一次
 ************************************************/
void All_PID_Init(void)
{
    //GPS和IMU PID
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
 *  函数名称: PID控制器更新
 *  函数说明:
 *  输入说明:
 *  输   出: PID结构体指针
 *  返回值:
 *  更新时间: 2026年4月22日
 *  备   注:
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
 *  函数名称: PID控制器更新
 *  函数说明: 目前调用所有PID控制器更新函数
 *  输入说明:
 *  输   出:
 *  返回值:
 *  更新时间: 2026年4月22日
 *  备   注: 需要在中断中调用，周期为10ms
 ************************************************/
void PID_Controller_Update(void)
{
    static int8 Timer = 0;

    /*=========================PID 10ms 更新================================*/

    //PID速度控制器
    PID_Controller_Update_Parameter(&PID_Right_Speed_Encoder);
    PID_Controller_Update_Parameter(&PID_Left_Speed_Encoder);

    /*=========================PID 20ms 更新================================*/
    Timer ++;
    if(Timer == 2)
    {
        Timer = 0;
        Angle_Encoder_PID_Update();    /*前轮角度达到后根据角度转向PID更新*/

        //读取编码器值
        Read_Encoder_Value_Update(TIM2_ENCODER);
        Read_Encoder_Value_Update(TIM5_ENCODER);

        //GPS和IMU转向控制
        PID_Controller_Update_Parameter(&PID_GPS_IMU);

        PID_Controller_Update_Parameter(&PID_Diversion);    //转向PID
        PID_Controller_Update_Parameter(&PID_Location);     //位置PID
        PID_Controller_Update_Parameter(&PID_Angle);        //角度PID
    }
}

/************************************************
 *  函数名称: 电机速度PID启动函数
 *  函数说明:
 *  输入说明: 范围+-180
 *  输   出: Start为Open 表示启动PID，Close 关闭PID
 *          Left_Target为 左轮PID目标值
 *          Right_Target为右轮PID目标值
 *  返回值:
 *  更新时间: 2026年4月22日
 *  备   注:
 ************************************************/
 void Motor_Speed_Encoder_PID_Start(PID_Start_enum Start,int32 Left_Target,int32 Right_Target)
 {
     PID_Left_Speed_Encoder.Start  = Start;
     PID_Right_Speed_Encoder.Start = Start;

     int16 Right_Encoder_Value = Unit_Time_Read_Encoder_Value(TIM2_ENCODER);    //右
     int16 Left_Encoder_Value  = Unit_Time_Read_Encoder_Value(TIM2_ENCODER);    //左

     PID_Left_Speed_Encoder.Actual  = (float)Left_Encoder_Value;
     PID_Left_Speed_Encoder.Target  = (float)Left_Target;

     PID_Right_Speed_Encoder.Actual = (float)Right_Encoder_Value;
     PID_Right_Speed_Encoder.Target = (float)Right_Target;
 }

 /************************************************
  *  函数名称: GPS和IMU方向PID启动函数
  *  函数说明:
  *  输入说明:
  *  输   出: Start为Open 表示启动PID，Close 关闭PID
  *          Actual为PID实际值
  *          Target为PID目标值
  *  返回值: PID控制器的输出结果
  *  更新时间: 2026年4月28日
  *  备   注: 支持正反方向，根据方向切换PID参数
  ************************************************/
 int32 GPS_IMU_PID_Start(PID_Start_enum Start,int32 Actual,int32 Target,int8 Direction)
 {
     static int8 last_direction = 1;

     /* 方向切换时清除积分累计，防止反向积分饱和 */
     if (Direction != last_direction)
     {
         PID_GPS_IMU.Iout = 0.0f;
         last_direction = Direction;
     }

     /* 根据方向动态切换PID参数 */
     if (Direction == -1)  /* 倒车：使用倒车专用参数 */
     {
         PID_GPS_IMU.Kp = GPS_IMU_REVERSE_KP;
         PID_GPS_IMU.Ki = GPS_IMU_REVERSE_KI;
         PID_GPS_IMU.Kd = GPS_IMU_REVERSE_KD;
     }
     else  /* 前进：使用默认参数 */
     {
         PID_GPS_IMU.Kp = GPS_IMU_PID_KP;
         PID_GPS_IMU.Ki = GPS_IMU_PID_KI;
         PID_GPS_IMU.Kd = GPS_IMU_PID_KD;
     }

     PID_GPS_IMU.Start  = Start;
     PID_GPS_IMU.Actual = Actual;
     PID_GPS_IMU.Target = Target;

     /* 倒车时对PID输出施加额外的增益倍率，补偿倒车转向效率低的问题 */
     if (Direction == -1)
     {
         return (int32)((float)PID_GPS_IMU.Result * GPS_IMU_REVERSE_GAIN);
     }

     return PID_GPS_IMU.Result;
 }

 /************************************************
  *  函数名称: 转向PID启动函数
  *  函数说明:
  *  输入说明:
  *  输   出: Start为Open 表示启动PID，Close 关闭PID
  *          Actual为PID实际值
  *          Target为PID目标值
  *  返回值: PID控制器的输出结果
  *  更新时间: 2026年4月28日
  *  备   注: 根据方向切换对应的PID
  ************************************************/
 int32 Diversion_PID_Start(PID_Start_enum Start,int32 Actual,int32 Target)
 {
     PID_Diversion.Start  = Start;
     PID_Diversion.Actual = Actual;
     PID_Diversion.Target = Target;

     return PID_Diversion.Result;
 }

 /************************************************
  *  函数名称: 位置PID启动函数
  *  函数说明:
  *  输入说明:
  *  输   出: Start为Open 表示启动PID，Close 关闭PID
  *          Actual为PID实际值
  *          Target为PID目标值
  *  返回值: PID控制器的输出结果
  *  更新时间: 2026年4月28日
  *  备   注: 根据方向切换对应的PID
  ************************************************/
 int32 Location_PID_Start(PID_Start_enum Start,int32 Actual,int32 Target)
{
    PID_Location.Start  = Start;
    PID_Location.Actual = Actual;
    PID_Location.Target = Target;

    return PID_Location.Result;
}

 /************************************************
  *  函数名称: 角度PID启动函数
  *  函数说明:
  *  输入说明:
  *  输   出: Start为Open 表示启动PID，Close 关闭PID
  *          Actual为PID实际值
  *          Target为PID目标值
  *  返回值: PID控制器的输出结果
  *  更新时间: 2026年4月28日
  *  备   注: 根据方向切换对应的PID
  ************************************************/
 int32 Angle_PID_Start(PID_Start_enum Start,int32 Actual,int32 Target)
{
    PID_Angle.Start  = Start;
    PID_Angle.Actual = Actual;
    PID_Angle.Target = Target;

    return PID_Angle.Result;
}

/*=========================以下PID调试部分 == 可删=================================*/
int CsSspeed;/*速度控制 速度的调试值 可删*/
float CsLocation;/*位置控制 位置的调试值 可删*/

/************************************************
 *  函数名称: PID数据显示
 *  函数说明:
 *  输入说明:
 *  输   出:
 *  返回值:
 *  更新时间: 2026年3月15日
 *  备   注: 调试时使用 需要时通过注释去掉打印位
 ************************************************/
void PID_DataShow(void)
{
//    printf("%d,%d,%d\n\r",(int)Motor,current_angle,(int)Target_Position);/*位置式输出值,调试值，可删*/
}

/************************************************
 *  函数名称: PID调试参数
 *  函数说明: 循环调试使用
 *  输入说明:
 *  输   出:
 *  返回值:
 *  更新时间: 2026年3月23日
 *  备   注: 调试使用
 ************************************************/
void PID_adjust_parameter(void)/*调试值，可删*/
{

}

/************************************************
 *  函数名称: PID调试参数
 *  函数说明: 中断中使用
 *  输入说明:
 *  输   出:
 *  返回值:
 *  更新时间: 2026年3月23日
 *  备   注: 调试使用
 ************************************************/
void All_PID_scan_adjust_parameter(void)/*调试值，可删*/
{

}

/*=========================以上PID调试部分===================================*/
