/*
 * IMU.h
 *
 *  Created on: 2025��4��16��
 *      Author: LENOVO
 */
/*
 * IMU.c
 *
 *  Created on: 2026��6��13��
 *      Author: LiangZhaoqi
 *      �����Ż�
 */
#ifndef CODE_IMU_H_
#define CODE_IMU_H_

#define IMU_660RA_Start     (1)     //1����IMU_660RA 0����IMU_963RA

#define IMU_660RA_KALMAN    (0)     //1����IMU_660RA �Ŀ�����

typedef struct{
    double Xdata;//��Ʈ����X
    double Ydata;//��Ʈ����Y
    double Zdata;//��Ʈ����Z
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
    float acc_x;   //x����ٶ�
    float acc_y;   //y����ٶ�
    float acc_z;   //z����ٶ�

    float gyro_x;  //x����ٶ�
    float gyro_y;  //y����ٶ�
    float gyro_z;  //z����ٶ�
}IMU_param_t ;

#endif

void IMU_init(void);
void Get_Angle(void);

#endif /* CODE_IMU_H_ */
