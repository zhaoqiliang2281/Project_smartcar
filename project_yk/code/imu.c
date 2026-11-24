/*
 * imu.c
 *
 *  Created on: 2025年6月12日
 *      Author: 吉祥止止
 */

#include "zf_common_headfile.h"


double Daty_Z = 0;//Z轴规划,规划为0-180和0-(-180)
double Angle=0;   //Z轴规划,规划为0-360和0-(-360)(备用1)

void IMU_Handle_180(void)//规划为0-180和0-(-180)   Dazy_Z
{
    Daty_Z=Angle;
    if((Daty_Z>0&&Daty_Z<=180)  ||   (Daty_Z<0&&Daty_Z>=(-180)))    { Daty_Z= +Daty_Z;}
    else if(Daty_Z>180)                                             { Daty_Z-=360;    }
    else if(Daty_Z<-180)                                            { Daty_Z+=360;    }
}


double angle;                //数据融合后的角度  //float
//acc_ratio决定着滤波算法对加速度计的信任度，如果越大则使用加速度计的分量越多，
//大了之后会出现在车模运动的过程中，由于加速度计受到运动而带来的噪声，从而导致滤波之后的曲线不够平滑，
//小了会出现跟踪加速度计的速度缓慢。
double acc_ratio=60.2; //6.4;      //加速度计比例9.597 10ms 1.401      6.4
//gyro_ratio决定着滤波算法对陀螺仪的信任度，
//如果越大则使用陀螺仪的分量越多，但是越大容易出现过冲的现象，
//过小则会出现响应速度不够的情况。
//float gyro_ratio=25.4; //4.6;    //陀螺仪比例3.290  4.605     4.6

double gyro_ratio=59.5;//25
double dt = 0.001;           //采样周期

double angle_calc(double angle_m, double gyro_m)
{
    double temp_angle;
    double gyro_now;
    double error_angle;
    static double last_angle;
    static uint8 first_angle;
    if(!first_angle)//判断是否为第一次运行本函数
    {  //如果是第一次运行，则将上次角度值设置为与加速度值一致
        first_angle = 1;
        last_angle = angle_m;
    }
    gyro_now = gyro_m * gyro_ratio;
    //根据测量到的加速度值转换为角度之后与上次的角度值求偏差
    error_angle = (angle_m - last_angle)*acc_ratio;
    //根据偏差与陀螺仪测量得到的角度值计算当前角度值
    temp_angle = last_angle + (error_angle + gyro_now)*dt;
    //保存当前角度值
    last_angle = temp_angle;
    return temp_angle;
}
double data=0;
double Acc_x,Gra_z;

double IMU_Get(void)
{
    imu660ra_get_acc();
    imu660ra_get_gyro();
    Acc_x = imu660ra_acc_transition(imu660ra_acc_x);
    Gra_z = imu660ra_gyro_transition(imu660ra_gyro_z);
    angle = angle_calc(Acc_x,Gra_z);
    return angle;
}

void get_angle(void)
{
    data = IMU_Get();
    if(data > 2 || data < -2)       {Angle+= - data*0.01;}
    if(Angle >360)                  {Angle = Angle - 360;}
    else if(Angle < -360)           {Angle = Angle + 360;}

    IMU_Handle_180();
}
