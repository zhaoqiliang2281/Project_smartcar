/*
 * IMU.h
 *
 *  Created on: 2025年4月16日
 *      Author: LENOVO
 */
/*
 * IMU.c
 *
 *  Created on: 2026年6月13日
 *      Author: LiangZhaoqi
 *      二次优化
 */
#ifndef CODE_IMU_H_
#define CODE_IMU_H_

#define IMU_660RA_Start     (1)     //1开启IMU_660RA 0开启IMU_963RA

#define IMU_660RA_KALMAN    (0)     //1开启IMU_660RA 的卡尔曼

typedef struct{
    double Xdata;//零飘参数X
    double Ydata;//零飘参数Y
    double Zdata;//零飘参数Z
}gyro_param_t;

extern double Roll,Pitch,Yaw;
extern double yaw;

#if IMU_660RA_Start

typedef struct {
    double q0;
    double q1;
    double q2;
    double q3;
    double roll;
    double pitch;
    double yaw;
    double beta;
} quaternion_t;

#else

typedef struct{
    float acc_x;   //x轴加速度
    float acc_y;   //y轴加速度
    float acc_z;   //z轴加速度

    float gyro_x;  //x轴角速度
    float gyro_y;  //y轴角速度
    float gyro_z;  //z轴角速度
}IMU_param_t ;

#endif

void IMU_init(void);
void Get_Angle(void);

#endif /* CODE_IMU_H_ */
