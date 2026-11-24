/*
 * IMU.c
 *
 *  Created on: 2025年4月16日
 *      Author: LENOVO
 */
/*
 * IMU.c
 *
 *  Created on: 2026年6月13日
 *      Author: LiangZhaoqi
 *      二次优化,使用IMU963的卡尔曼滤波，增加了IMU6轴的陀螺仪的卡尔曼滤波，并加注释
 */
#include "zf_common_headfile.h"

#define DEG_TO_RAD   0.017453292519943f

double Roll,Pitch,Yaw;//陀螺仪解算出的数据
double yaw;

gyro_param_t Gyro_Offset;//零飘参数

#if IMU_660RA_Start
/*
 * ********************** 六轴IMU解算 **********************
  __     ____   ___  _____   ___ __  __ _   _
 / /_   |  _ \ / _ \|  ___| |_ _|  \/  | | | |
| '_ \  | | | | | | | |_     | || |\/| | | | |
| (_) | | |_| | |_| |  _|    | || |  | | |_| |
 \___/  |____/ \___/|_|     |___|_|  |_|\___/

 * ********************** 六轴IMU解算 **********************
 */
static const double dt = 0.001f;  // 1ms周期
static uint8 initialized = 0;

static quaternion_t attitude = {
        .q0=1.0f,
        .q1=0.0f,
        .q2= 0.0f,
        .q3= 0.0f,
        .beta = 0.1f
};

/************************************************
 *  函数名称：IMU_quaternion_init
 *  功能说明：初始化四元数姿态估计参数
 *  输入参数：无
 *  输出参数：重置姿态四元数和初始化标志
 *  修改时间：2026年6月11日
 *  备注：用于 IMU_660RA 模式下的姿态初始化
 ************************************************/
static void IMU_quaternion_init(void)
{
    attitude.q0 = 1.0f;
    attitude.q1 = attitude.q2 = attitude.q3 = 0.0f;
    initialized = 1;
}

/************************************************
 *  函数名称：normalize_vector
 *  功能说明：对三轴加速度向量进行归一化处理
 *  输入参数：x/y/z 三轴向量指针
 *  输出参数：归一化后的三轴向量
 *  修改时间：2026年6月11日
 *  备注：内部辅助函数，供四元数姿态更新使用
 ************************************************/
static void normalize_vector(double *x, double *y, double *z)
{
    double recipNorm = 1.0f / sqrtf(*x * *x + *y * *y + *z * *z);
    *x *= recipNorm;
    *y *= recipNorm;
    *z *= recipNorm;
}

static uint8 Angle_flag = 0;

/************************************************
 *  函数名称：get_euler_angles
 *  功能说明：获取当前姿态四元数对应的欧拉角数据
 *  输入参数：无
 *  输出参数：返回姿态结构体指针
 *  修改时间：2026年6月11日
 *  备注：供上层调用读取 Roll/Pitch/Yaw
 ************************************************/
const quaternion_t* get_euler_angles(void)
{
    return &attitude;
}

/************************************************
 *  函数名称：quaternion_update
 *  功能说明：基于加速度计和陀螺仪更新四元数姿态
 *  输入参数：无
 *  输出参数：更新全局姿态角数据
 *  修改时间：2026年6月11日
 *  备注：IMU_660RA 模式下的核心姿态解算函数static
 ************************************************/
void imu660ra_quaternion_update(void)
{
    if(!initialized) return;

    imu660ra_get_acc();         // 新增加速度计采集
    imu660ra_get_gyro();        // 采集陀螺仪数据

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

    const quaternion_t* angle = get_euler_angles();
    Roll  = angle->roll;
    Pitch = angle->pitch;
    Yaw   = angle->yaw;
    yaw   = Yaw;
    if(Yaw<0)  { Yaw+=360;}

    if(Angle_flag ==1)
    {
        IMU_quaternion_init();
        Angle_flag =0;
    }
}

void imu660ra_kalman_update(void)
{
    imu660ra_get_acc();
    imu660ra_get_gyro();

    Axis3f acc = {
        .x = imu660ra_acc_transition(imu660ra_acc_x),
        .y = imu660ra_acc_transition(imu660ra_acc_y),
        .z = imu660ra_acc_transition(imu660ra_acc_z)
    };

    Axis3f gyro = {
        .x = imu660ra_gyro_transition(imu660ra_gyro_x),
        .y = imu660ra_gyro_transition(imu660ra_gyro_y),
        .z = imu660ra_gyro_transition(imu660ra_gyro_z)
    };

    imu_update(acc, gyro, 0.001f);

    EulerAngles angles = imu_get_euler_angles(gyro);
    Roll  = angles.roll;
    Pitch = angles.pitch;
    Yaw   = angles.yaw;
    yaw   = Yaw;
    if(Yaw < 0.0f)
    {
        Yaw += 360.0f;
    }
}

#else
/*
 * ********************** 九轴IMU解算 **********************
  ___    ____   ___  _____   ___ __  __ _   _
 / _ \  |  _ \ / _ \|  ___| |_ _|  \/  | | | |
| (_) | | | | | | | | |_     | || |\/| | | | |
 \__, | | |_| | |_| |  _|    | || |  | | |_| |
   /_/  |____/ \___/|_|     |___|_|  |_|\___/

 * ********************** 九轴IMU解算 **********************
 */
IMU_param_t  IMU_Data;

/************************************************
 *  函数名称：IMU_gyro_Offset_Init
 *  功能说明：采样陀螺仪静止时的零漂值，用于后续去漂
 *  输入参数：无
 *  输出参数：更新 Gyro_Offset 全局零漂参数
 *  修改时间：2026年6月11日
 *  备注：初始化过程中调用，保证角速度计算精度
 ************************************************/
static void IMU_gyro_Offset_Init(void)
{
    Gyro_Offset.Xdata = 0;
    Gyro_Offset.Ydata = 0;
    Gyro_Offset.Zdata = 0;
    for (uint16_t i = 0; i < 1000; i++)
    {
        Gyro_Offset.Xdata += imu963ra_gyro_x;
        Gyro_Offset.Ydata += imu963ra_gyro_y;
        Gyro_Offset.Zdata += imu963ra_gyro_z;
        system_delay_ms(5);   // 最大 1Khz
    }

    Gyro_Offset.Xdata /= 1000;
    Gyro_Offset.Ydata /= 1000;
    Gyro_Offset.Zdata /= 1000;
}

/************************************************
 *  函数名称：imu_kalman_quaternion_update
 *  功能说明：采用卡尔曼融合方式更新姿态角
 *  输入参数：无
 *  输出参数：更新全局 Yaw 角度
 *  修改时间：2026年6月11日
 *  备注：IMU_963RA 模式下的姿态更新函数
 ************************************************/
static void imu_kalman_quaternion_update(void)
{
    imu963ra_get_gyro();
    imu963ra_get_acc();
    Axis3f acc, gyro;
    // 加速度值转换
    // 在±8g量程下，获取到的加速度计数据除以 4098   可以转化为带物理单位的数据 单位 g(m/s^2)
    acc.x = (float)imu963ra_acc_x / 4098.0f;
    acc.y = (float)imu963ra_acc_y / 4098.0f;
    acc.z = (float)imu963ra_acc_z / 4098.0f;

    // 陀螺仪值转换
    // 陀螺仪量程为 ±2000 dps 获取到的陀螺仪数据除以 14.3    可以转化为带物理单位的数据 单位为 °/s
    gyro.x = (float)imu963ra_gyro_x / 14.3f;
    gyro.y = (float)imu963ra_gyro_y / 14.3f;
    gyro.z = (float)imu963ra_gyro_z / 14.3f;

    // 更新姿态估计，dt=0.005秒 (200Hz采样率)
    imu_update(acc, gyro, 0.001f);

    // 获取欧拉角结果
    EulerAngles angles = imu_get_euler_angles(gyro);

    // 更新全局欧拉角变量以供使用
    //roll_Kalman = angles.roll;
    //pitch_Kalman = angles.pitch;
    Yaw = -angles.yaw;
    if(Yaw<0)//规划为0-360和0-(-360)
    {
        Yaw+=360.0f;
    }

}

#endif

/************************************************
 *  函数名称：IMU_init
 *  功能说明：初始化 IMU 传感器并完成零漂校准准备
 *  输入参数：无
 *  输出参数：无
 *  修改时间：2026年6月11日
 *  备注：根据 IMU_660RA_Start 宏切换不同 IMU 驱动
 ************************************************/
void IMU_init(void)
{

#if IMU_660RA_Start

    imu660ra_init();            //陀螺仪初始化
    IMU_quaternion_init();      //四元素解算初始化
    imu_init();                 // 复用 IMU963 的滤波状态初始化

#else

    imu963ra_init();            //IMU963ra惯导初始化
    IMU_gyro_Offset_Init();     // 陀螺仪零漂初始化

#endif

}

/************************************************
 *  函数名称：Get_Angle
 *  功能说明：获取当前姿态角信息（Roll、Pitch、Yaw）
 *  输入参数：无
 *  输出参数：更新全局姿态角变量
 *  修改时间：2026年6月11日
 *  备注：定时任务中调用，用于外部姿态控制
 ************************************************/
void Get_Angle(void)
{

#if IMU_660RA_Start

#if IMU_660RA_KALMAN

    imu660ra_kalman_update();

#else

    imu660ra_quaternion_update();

#endif

#else

    imu_kalman_quaternion_update();

#endif

}
//测试
//printf("%f\n\r",Yaw);//上位机看值
