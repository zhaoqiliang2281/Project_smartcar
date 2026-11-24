/*
 * quaternion.c
 *
 *  Created on: 2025年6月12日
 *      Author: LiangZhaoqi
 */

#include "zf_common_headfile.h"

#define DEG_TO_RAD   0.017453292519943f
static const double dt = 0.001f;  // 1ms周期
static uint8 initialized = 0;

gyro_param_t Gyro_Offset;//零飘参数

double angle1 = 0;

int Angle_flag = 0;

double Roll,Pitch,Yaw;//陀螺仪解算出的数据

//double Yaw_angle = 0;

static quaternion_t attitude = {
        .q0=1.0f,
        .q1=0.0f,
        .q2= 0.0f,
        .q3= 0.0f,
        .beta = 0.1f
};

void quaternion_init(void)
{
    attitude.q0 = 1.0f;
    attitude.q1 = attitude.q2 = attitude.q3 = 0.0f;
    initialized = 1;
}

void imu_calibration1(void)
{
    Gyro_Offset.Xdata = 0;
    Gyro_Offset.Ydata = 0;
    Gyro_Offset.Zdata = 0;
    for(int i=0;i<1000;i++)
    {
        Gyro_Offset.Xdata += imu660ra_gyro_x;//三轴陀螺仪数据
        Gyro_Offset.Ydata += imu660ra_gyro_y;
        Gyro_Offset.Zdata += imu660ra_gyro_z;
        system_delay_ms(1);
    }
    Gyro_Offset.Xdata /=1000;
    Gyro_Offset.Ydata /=1000;
    Gyro_Offset.Zdata /=1000;
}

// 加速度归一化
static void normalize_vector(double *x, double *y, double *z)
{
    double recipNorm = 1.0f / sqrtf(*x * *x + *y * *y + *z * *z);
    *x *= recipNorm;
    *y *= recipNorm;
    *z *= recipNorm;
}



void quaternion_update(void)
{
    if(!initialized) return;

    // 获取传感器数据
    double gx = imu660ra_gyro_transition(imu660ra_gyro_x/10*10) * DEG_TO_RAD;
    double gy = imu660ra_gyro_transition(imu660ra_gyro_y/10*10) * DEG_TO_RAD;
    double gz = imu660ra_gyro_transition(imu660ra_gyro_z/10*10)* DEG_TO_RAD;

    double ax = imu660ra_acc_transition(imu660ra_acc_x/100*100);
    double ay = imu660ra_acc_transition(imu660ra_acc_y/100*100);
    double az = imu660ra_acc_transition(imu660ra_acc_z/100*100);
    normalize_vector(&ax, &ay, &az);
    // 当前姿态预测的重力向量
    double vx = 2.0f * (attitude.q1*attitude.q3 - attitude.q0*attitude.q2);
    double vy = 2.0f * (attitude.q0*attitude.q1 + attitude.q2*attitude.q3);
    double vz = attitude.q0*attitude.q0 - attitude.q1*attitude.q1
             - attitude.q2*attitude.q2 + attitude.q3*attitude.q3;

    // 计算误差
    double ex = (ay*vz - az*vy);
    double ey = (az*vx - ax*vz);
    double ez = (ax*vy - ay*vx);

    // 应用补偿
    gx += attitude.beta * ex;
    gy += attitude.beta * ey;
    gz += attitude.beta * ez;
    // 四元数更新
    double q0 = attitude.q0;
    double q1 = attitude.q1;
    double q2 = attitude.q2;
    double q3 = attitude.q3;

    attitude.q0 += (-q1*gx - q2*gy - q3*gz) * 0.5f * dt;
    attitude.q1 += ( q0*gx - q3*gy + q2*gz) * 0.5f * dt;
    attitude.q2 += ( q3*gx + q0*gy - q1*gz) * 0.5f * dt;
    attitude.q3 += (-q2*gx + q1*gy + q0*gz) * 0.5f * dt;

    // 归一化处理
    double norm = sqrtf(attitude.q0*attitude.q0 + attitude.q1*attitude.q1 + attitude.q2*attitude.q2 + attitude.q3*attitude.q3);

    norm = 1.0f / norm;
    attitude.q0 *= norm;
    attitude.q1 *= norm;
    attitude.q2 *= norm;
    attitude.q3 *= norm;

    // 转换为欧拉角（弧度）
    attitude.roll = atan2f(2*(q0*q1 + q2*q3), 1 - 2*(q1*q1 + q2*q2))*(1/DEG_TO_RAD);
    attitude.pitch = asinf(2*(q0*q2 - q3*q1))*(1/DEG_TO_RAD);
    attitude.yaw = -atan2f(2*(q0*q3 + q1*q2), 1 - 2*(q2*q2 + q3*q3))*(1/DEG_TO_RAD);
    if(Angle_flag ==1)
    {
        quaternion_init();
        Angle_flag =0;
    }
}

const quaternion_t* get_euler_angles(void)
{
    return &attitude;
}

void Get_Angle(void)
{
    const quaternion_t* angle = get_euler_angles();
    Roll  = angle->roll;
    Pitch = angle->pitch;
    Yaw   = angle->yaw;
    if(Yaw<0)  { Yaw+=360;}

}


