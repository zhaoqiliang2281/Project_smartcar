/*
 * PID.c
 *
 *  Created on: 2025年6月15日
 *      Author: LiangZhaoqi
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

/*********************************PID运算代码部分******************************************/

pid_type_def PID_Servo;

int16 resultServo;  //舵机PID计算结果

//速度PID
pid_type_def PID_Speed;

int16 TargetSpeed=0;//目标速度
int16 practicalSpeed;//实际速度
int16 resultSpeed;//PID计算结果
int16 resultSpeed1;//PID计算结果

//位置PID
pid_type_def PID_Location;

int16 Target_Location=0;//目标距离
int16 practica_Location;//实际距离
int16 result_Location;//PID计算结果

/************************************************
 *  函数名称: 全部PID初始化：All_PID_Init(void)
 *  功能说明：速度与位置PID串级
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月17日
 *  备   注：开始时使用，只调用一次
 ************************************************/
void All_PID_Init(void)
{
    //速度PID
    float SpeedPID_Value[3]={Speed_pid_KP,Speed_pid_KI,Speed_pid_KD};
    PID_init(&PID_Speed,PID_POSITION,SpeedPID_Value,20,50); //速度限幅20

    //位置PID
//    float LocationPID_Value[3]={Llocation_pid_KP,Llocation_pid_KI,Llocation_pid_KD};
//    PID_init(&PID_Location,PID_POSITION,LocationPID_Value,120,100);
    float LocationPID_Value[3]={Llocation_pid_KP,Llocation_pid_KI,Llocation_pid_KD};
    PID_init(&PID_Location,PID_POSITION,LocationPID_Value,350,100);//230->60
}

/************************************************
 *  函数名称: 速度PID重新初始化：Speed_PID_Init(void)
 *  功能说明：
 *  参数说明：速度限幅输出
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年7月10日
 *  备   注：在科目中，点位中调用一次重新设置速度限幅值
 ************************************************/
void Speed_PID_Init(int8 num)
{
    //速度PID
    float SpeedPID_Value[3]={Speed_pid_KP,Speed_pid_KI,Speed_pid_KD};
    PID_init(&PID_Speed,PID_POSITION,SpeedPID_Value,num,50); //速度限幅
}
void Speed_Location_Init(int16 num)
{
    //速度PID
    float LocationPID_Value[3]={Llocation_pid_KP,Llocation_pid_KI,Llocation_pid_KD};
    PID_init(&PID_Location,PID_POSITION,LocationPID_Value,num,100);
}
/************************************************
 *  函数名称: 摄像头舵机初始化：Servo_PID_Init(void)
 *  功能说明：
 *  参数说明：pid参数
 *  参   数：KP,KI,KD
 *  函数返回：无
 *  修改时间：2025年7月10日
 *  备   注：
 ************************************************/
void servo_PID_Init(float Servo_pid_KP,float Servo_pid_KI,float Servo_pid_KD)
{
    //舵机PID
    float ServoPID_Value[3]={Servo_pid_KP,Servo_pid_KI,Servo_pid_KD};
    PID_init(&PID_Servo,PID_POSITION,ServoPID_Value,20,100); //舵机角度为75-95-115，差值为20，限幅20，
}

/************************************************
 *  函数名称: PID参数显示：PID_DataShow(void)
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月17日
 *  备   注：调参使用 跑车是看的数据
 ************************************************/
void PID_DataShow(void)
{
//    ips200_show_int(  150 , 16*16,  result_Location,    8); //位置PID计算结果
//    ips200_show_int(  150 , 16*17, resultSpeed ,    8);     //速度PID计算结果
//    ips200_show_int(  150 , 16*18,  encoder[0],    8);      //实际编码速度
//    ips200_show_float(  100 , 16*19,  Distance,4,6);        //两点之间的距离
//    printf("%d,%d,%d,%d,%d\n\r",encoder[0],result_Location,resultSpeed,next,(int)Distance);
    printf("%d,%d,%d\n\r",result_Location,resultSpeed,next);//上位机看值
}

/************************************************
 *  函数名称: PID计算：All_PID_scan(void)
 *  功能说明：放在中断调用：时间间隔：10ms
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月17日
 *  备   注：
 ************************************************/
void All_PID_scan(void)
{
    //舵机PID计算结果
    resultServo = PID_calc(&PID_Servo,angle_Error,0);

    //GPS位置PID计算结果
    result_Location=PID_calc(&PID_Location,Distance,0);

    //电机速度PID计算结果
    resultSpeed=PID_calc(&PID_Speed,encoder[0],result_Location);
}

/*=========================以下PID调参代码 == 勿删=================================*/

int CsSspeed;/*速度环测试 速度调参代码 勿删*/
float CsLocation;/*位置环测试 位置调参代码 勿删*/
void PID_adjust_parameter(void)/*速度调参代码，勿删*/
{
    /*速度调参代码,勿删*/
/*
    if(key1_flag==1)
    {
        key1_flag=0;
        Buzzer_check(50);
        CsSspeed=10;
    }
    if(key2_flag==1)
    {
        key2_flag=0;
        Buzzer_check(50);
        CsSspeed=20;
    }
    if(key3_flag==1)
    {
        key3_flag=0;
        Buzzer_check(50);
        CsSspeed=30;
    }
    if(key4_flag==1)
    {
        key4_flag=0;
        Buzzer_check(50);
        CsSspeed=50;
    }
    if(key5_flag==1)
    {
        key5_flag=0;
        Buzzer_check(50);
        CsSspeed=80;
    }
    printf("%d,%d,%d\n\r",(int)encoder[0],resultSpeed,CsSspeed);  //给模拟速度测试
                              //实际编码速度，速度PID计算结果，目标编码速度
    ips200_show_int(  150 , 16*17, encoder[0] , 8); //实际编码速度
    ips200_show_int(  150 , 16*18, resultSpeed , 8);//速度PID计算结果
    ips200_show_float( 100 , 16*19, CsSspeed,4,6);     //目标编码速度
*/
    /*位置调参代码，勿删*/
    if(key1_flag == 1)
    {
        key1_flag = 0;
        Buzzer_SetTimer(50);
        ips200_show_float(  130 , 16*6, 66 ,4,6); //实际编码速度

    }
    if(key2_flag == 1)
    {
        key2_flag = 0;
        Buzzer_SetTimer(50);
    }
    if(key3_flag == 1)
    {
        key3_flag = 0;
        Buzzer_SetTimer(50);
    }
    if(key4_flag == 1)
    {
        key4_flag = 0;
        Buzzer_SetTimer(100);
        ips200_show_float(  130 , 16*6, 10 ,4,6); //实际编码速度
    }
    if(key5_flag == 1)
    {
        key5_flag = 0;
        Buzzer_SetTimer(200);
    }

//    if(key1_flag == 1)
//    {
//        key1_flag = 0;
//        Buzzer_check(50);
//        CsLocation += 1.0;
//    }
//    if(key2_flag == 1)
//    {
//        key2_flag = 0;
//        Buzzer_check(50);
//        CsLocation -= 1.0;
//    }
//    if(key3_flag == 1)
//    {
//        key3_flag = 0;
//        Buzzer_check(50);
//        CsLocation = 3.0;
//    }
//    if(key4_flag == 1)
//    {
//        key4_flag = 0;
//        Buzzer_check(50);
//        CsLocation = 4.0;
//    }
//    if(key5_flag == 1)
//    {
//        key5_flag = 0;
//        Buzzer_check(50);
//        CsLocation = 0;
//    }

    printf("%d,%d,%d\n\r",(int)CsLocation,result_Location,0);
                              //目标距离大小(设置为0)，位置PID计算结果，目标编码速度

    ips200_show_float(  130 , 16*10, encoder[0] ,4,6); //实际编码速度
    ips200_show_float(  130 , 16*11, resultSpeed ,4,6);//速度PID计算结果

    ips200_show_float(  130 , 16*13, CsLocation , 4,6);  //实际位置距离
    ips200_show_float(  130 , 16*14, result_Location ,4,6); //位置PID计算结果

    ips200_show_float( 130 , 16*15, 0,4,6);   //目标0

    Pwm_Motor(resultSpeed);
}

void All_PID_scan_adjust_parameter(void)/*速度调参代码，勿删*/
{
    //GPS位置PID计算结果
    result_Location=PID_calc(&PID_Location,(float)CsLocation,0);    //给模拟距离测试

    //电机速度PID计算结果
    resultSpeed=PID_calc(&PID_Speed,encoder[0],(float)result_Location);//CsSspeed
}

/*=========================上面PID调参代码===================================*/

