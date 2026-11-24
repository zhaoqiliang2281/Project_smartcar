/*
 * PID.c
 *
 *  Created on: 2026年3月15日
 *      Author: 吉祥止止
 */

#include "zf_common_headfile.h"

/*********************************PID公式运算代码部分******************************************/

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

/**
  * @brief          pid struct data init
  * @param[out]     pid: PID结构数据指针
  * @param[in]      mode: PID_POSITION:普通PID
  *                 PID_DELTA: 差分PID
  * @param[in]      PID: 0: kp, 1: ki, 2:kd
  * @param[in]      max_out: pid最大输出
  * @param[in]      max_iout: pid最大积分输出
  * @retval         none
  */
/************************************************
 *  函数名称: pid计算：PID_init(void)
 *  功能说明：
 *  参数说明：
 *  参   数：pid: PID结构数据指针; mode: PID_POSITION:普通PID,PID_DELTA: 差分PID;  PID: 0: kp, 1: ki, 2:kd;max_out: pid最大输出;max_iout: pid最大积分输出
 *  函数返回：无
 *  修改时间：2025年6月16日
 *  备   注：
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
    pid->error[0] = pid->error[1] = pid->error[2] = pid->Pout = pid->Iout = pid->Dout = pid->out = 0.0f;
}


/************************************************
 *  函数名称: pid计算：fp32 PID_calc(pid_type_def *pid, fp32 ref, fp32 set)
 *  功能说明：
 *  参数说明：
 *  参   数：pid: PID结构数据指针; ref: 反馈数据; set: 设定值
 *  函数返回：pid输出
 *  修改时间：2025年6月16日
 *  备   注：
 ************************************************/
fp32 PID_calc(pid_type_def *pid, fp32 ref, fp32 set)
{
    if (pid == NULL)
    {
        return 0.0f;
    }

    pid->error[2] = pid->error[1];
    pid->error[1] = pid->error[0];
    pid->set = set;
    pid->fdb = ref;
    pid->error[0] = set - ref;
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
        LimitMax(pid->out, pid->max_out);
    }
    return pid->out;
}


/************************************************
 *  函数名称: pid 输出清除：PID_clear(void)
 *  功能说明：
 *  参数说明：
 *  参   数：PID结构数据指针
 *  函数返回：无
 *  修改时间：2025年6月16日
 *  备   注：
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

/*********************************以上PID运算代码部分******************************************/

/*********************************以下前轮位置环PID转向部分******************************************/

/************************************************
 *  函数名称：方向电机转向位置式PID控制器：int32 Dir_Motor_Position_PID(int32 reality,int32 target)
 *  功能说明：
 *  参数说明：
 *  参   数：实际位置，目标位置
 *  函数返回：电机PWM
 *  修改时间：2026年1月16日
 *  备    注：根据位置式离散PID公式 pwm=Kp*e(k)+Ki*∑e(k)+Kd[e（k）-e(k-1)]
 ************************************************/
//float Position_KP = 18.95f;
//float Position_KI = 0.25f;
//float Position_KD = 0.1;

//12V
//float Position_KP = 58.95f;
//float Position_KI = 5.45f;
//float Position_KD = 0.01f;

//24V
float Position_KP = 28.95f;
float Position_KI = 0;//2.45f;
float Position_KD = 0.25f;


int32_t Dir_Motor_Position_PID_Control(int16 reality,int16 target)
{
    static float Bias = 0,Pwm = 0,Last_Bias = 0,Integral_bias = 0;

    Bias = target - reality;                        /* 计算偏差 */
    Integral_bias += Bias;                            /* 偏差累积 */

    if(Integral_bias > 2000) Integral_bias = 2000;   /* 积分限幅 */
    if(Integral_bias < -2000) Integral_bias = -2000;

    Pwm = (Position_KP*Bias)                        /* 比例环节 */
         +(Position_KI*Integral_bias)               /* 积分环节 */
         +(Position_KD*(Bias-Last_Bias));           /* 微分环节 */

    Last_Bias = Bias;                                 /* 保存上次偏差 */
    return Pwm;                                     /* 输出结果 */
}

int32 Motor = 0;            //PID计算出的电机PWM值
int16 current_angle;        //转换的角度值
int32 Target_Position = 0;  //目标位置
int16 Encoder_cnt = 0;      //累计编码值
/************************************************
 *  函数名称：定时器计算编码器累积值与转向电机PID：void Dir_Motor_Encoder_PID_Update(void)
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年1月15日
 *  备    注：中断10ms调用
 ************************************************/
void Dir_Motor_Encoder_PID_Update(void)
{
    Encoder_cnt += Read_Encoder(TIM4_ENCODER);          //读编码值

    current_angle = 4*Encoder_cnt;    //4倍频

    int16 error = current_angle - Target_Position;// 计算误差

    if(abs(error) < 1 && abs(Motor) < 10)       // 角度误差小于1°
    {
        Dir_Motor_SetPwm(0);                  //关闭电机
        Motor = 0;
    }
    else
    {
        Motor = (int32)Dir_Motor_Position_PID_Control(current_angle, (int16)Target_Position);    //PID计算
        Dir_Motor_SetPwm(Motor);
    }
//    printf("%d,%d,%d\n\r",(int)Motor,current_angle,(int)Target_Position);/*上位机看值,调参代码，勿删*/
}
/*********************************以上前轮位置环PID转向部分******************************************/
int32_t Dir_Motor_Position_PID_Control2(int16 reality,int16 target)
{
    static float Error = 0,Pwm = 0,Last_Error = 0,Integral_Error = 0;

    Error = target - reality;                            /* 计算偏差 */
    Integral_Error += Error;                              /* 偏差累积 */

    /*误差积分+积分分离*/

    if (fabs(Error) < 10)      //如果当前误差值小于指定的阈值
    {
        Integral_Error += Error;     //才进行正常的误差积分
    }
    else                        //否则，即当前误差值过大
    {
        Integral_Error = 0;           //此时不进行积分，同时把误差积分直接清零
    }

    if(Integral_Error > 2000) Integral_Error = 2000;      /* 积分限幅 */
    if(Integral_Error < -2000) Integral_Error = -2000;

    Pwm = (Position_KP*Error)                            /* 比例环节 */
         +(Position_KI*Integral_Error)                   /* 积分环节 */
         +(Position_KD*(Error-Last_Error));               /* 微分环节 */

    Last_Error = Error;                                   /* 保存上次偏差 */
    return Pwm;                                         /* 输出结果 */

}

/*********************************以下后轮PID部分******************************************/
//PID参数定义
pid_type_Parameter Speed_PID;
pid_type_Parameter Location_PID;
pid_type_Parameter Diversion_PID;
pid_type_Parameter Angle_PID;

//转向PID
pid_type_def PID_Diversion;

//速度PID
pid_type_def PID_Speed;

//位置PID
pid_type_def PID_Location;

//角度PID
pid_type_def PID_Angle;

/************************************************
 *  函数名称: 全部PID初始化：All_PID_Init(void)
 *  功能说明：速度与位置PID串级
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年3月15日
 *  备   注：初始化使用，只调用一次
 ************************************************/
void All_PID_Init(void)
{
    //转向PID
    float Diversion_PID_Value[3]={Diversion_pid_KP,Diversion_pid_KI,Diversion_pid_KD};
    PID_init(&PID_Diversion,PID_POSITION,Diversion_PID_Value,800,200); //速度限幅

    //速度PID
    float SpeedPID_PID_Value[3]={Speed_pid_KP,Speed_pid_KI,Speed_pid_KD};
    PID_init(&PID_Speed,PID_POSITION,SpeedPID_PID_Value,800,300); //速度限幅

    //位置PID
    float Location_PID_Value[3]={Llocation_pid_KP,Llocation_pid_KI,Llocation_pid_KD};
    PID_init(&PID_Location,PID_POSITION,Location_PID_Value,300,100);

    //角度PID
    float Angle_PID_Value[3]={Angle_pid_KP,Angle_pid_KI,Angle_pid_KD};
    PID_init(&PID_Angle,PID_POSITION,Angle_PID_Value,300,100);

}

/************************************************
 *  函数名称: PID计算：All_PID_scan(void)
 *  功能说明：放在中断调用：时间间隔：10ms
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年3月15日
 *  备   注：
 ************************************************/
//extern int32 Read_Encodernum2;

void PID_Controller_Update(void)
{
    static int8 Timer = 0;

    //转向PID
    if(Diversion_PID.Start == 1)
    {
        Diversion_PID.Result = PID_calc(&PID_Diversion,(float)Diversion_PID.Actual,(float)Diversion_PID.Target);
    }
    //速度PID
    if(Speed_PID.Start == 1)
    {
        Speed_PID.Result=PID_calc(&PID_Speed,(float)Speed_PID.Actual,(float)Speed_PID.Target);
    }
    //位置PID
    if(Location_PID.Start == 1)
    {
        Location_PID.Result=PID_calc(&PID_Location,(float)Location_PID.Actual,(float)Location_PID.Target);
    }
    //角度PID
    if(Angle_PID.Start == 1)
    {
        Angle_PID.Result=PID_calc(&PID_Angle,(float)Angle_PID.Actual,(float)Angle_PID.Target);
    }
    /*=========================PID20ms调用================================*/
    Timer ++;
    if(Timer == 2)
    {
        Timer = 0;
    }
}

/************************************************
 *  函数名称: void Diversion_PID_Start(uint8 Start,int32 Actual,int32 Target)
 *  功能说明：
 *  参数说明：
 *  参   数：Start：1表示开启PID，0关闭PID
 *         Actual：PID实际值
 *         Target：PID目标值
 *  函数返回：无
 *  修改时间：2026年3月18日
 *  备   注：
 ************************************************/
void Diversion_PID_Start(uint8 Start,int32 Actual,int32 Target)
{
    Diversion_PID.Start  = Start;
    Diversion_PID.Actual = Actual;
    Diversion_PID.Target = Target;
}

/************************************************
 *  函数名称: void Angle_PID_Start(uint8 Start,int32 Actual,int32 Target)
 *  功能说明：
 *  参数说明：
 *  参   数：Start：1表示开启PID，0关闭PID
 *         Actual：PID实际值
 *         Target：PID目标值
 *  函数返回：无
 *  修改时间：2026年3月18日
 *  备   注：
 ************************************************/
void Angle_PID_Start(uint8 Start,int32 Actual,int32 Target)
{
    Angle_PID.Start  = Start;
    Angle_PID.Actual = Actual;
    Angle_PID.Target = Target;
}

/*********************************以上后轮PID部分******************************************/

/*********************************以下上科三PID部分******************************************/

//PID定义
pid_type_def PID_YDir_Speed;
pid_type_def PID_DDir_Speed;

//PID参数定义
pid_type_Parameter YDir_Speed_PID;
pid_type_Parameter DDir_Speed_PID;

/************************************************
 *  函数名称: 全部PID初始化：KM3_All_PID_Init(void)
 *  功能说明：速度与位置PID串级
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年3月28日
 *  备   注：初始化使用，只调用一次
 ************************************************/
void KM3_All_PID_Init(void)
{
    //左右PID
    float YSpeedPID_PID_Value[3]={YSpeed_pid_KP,YSpeed_pid_KI,YSpeed_pid_KD};
    PID_init(&PID_YDir_Speed,PID_POSITION,YSpeedPID_PID_Value,800,100); //速度限幅

    //距离PID
    float DSpeedPID_PID_Value[3]={DSpeed_pid_KP,DSpeed_pid_KI,DSpeed_pid_KD};
    PID_init(&PID_DDir_Speed,PID_POSITION,DSpeedPID_PID_Value,800,100); //速度限幅

}
void KM3_PID_Controller_Update(void)
{
    //左右PID
    if(YDir_Speed_PID.Start == 1)
    {
        YDir_Speed_PID.Result=PID_calc(&PID_YDir_Speed,(float)YDir_Speed_PID.Actual,(float)YDir_Speed_PID.Target);
    }
    //距离PID
    if(DDir_Speed_PID.Start == 1)
    {
        DDir_Speed_PID.Result=PID_calc(&PID_DDir_Speed,(float)DDir_Speed_PID.Actual,(float)DDir_Speed_PID.Target);
    }
}
void YDir_Speed_PID_Start(uint8 Start,int32 Actual,int32 Target)
{
    YDir_Speed_PID.Start  = Start;
    YDir_Speed_PID.Actual = Actual;
    YDir_Speed_PID.Target = Target;
}
void DDir_Speed_PID_Start(uint8 Start,int32 Actual,int32 Target)
{
    DDir_Speed_PID.Start  = Start;
    DDir_Speed_PID.Actual = Actual;
    DDir_Speed_PID.Target = Target;
}
/*********************************以上科三PID部分******************************************/

/*=========================以下PID调参代码 == 勿删=================================*/
int CsSspeed;/*速度环测试 速度调参代码 勿删*/
float CsLocation;/*位置环测试 位置调参代码 勿删*/

/************************************************
 *  函数名称: PID参数显示：PID_DataShow(void)
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年3月15日
 *  备   注：调参使用 跑车时看的数据,通过串口传到上位机
 ************************************************/
void PID_DataShow(void)
{
//    printf("%d,%d,%d\n\r",Speed_PID.Actual,Speed_PID.Target,Speed_PID.Result);//转向PID测试
    printf("%d,%d,%d\n\r",(int)Motor,current_angle,(int)Target_Position);/*上位机看值,调参代码，勿删*/

}

/************************************************
 *  函数名称: PID调参代码：PID_adjust_parameter(void)
 *  功能说明：主循环调用
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年3月23日
 *  备   注：调参使用
 ************************************************/
void PID_adjust_parameter(void)/*调参代码，勿删*/
{

}

/************************************************
 *  函数名称: PID调参代码：PID_adjust_parameter(void)
 *  功能说明：中断调用
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2026年3月23日
 *  备   注：调参使用
 ************************************************/
void All_PID_scan_adjust_parameter(void)/*调参代码，勿删*/
{

}

/*=========================上面PID调参代码===================================*/






