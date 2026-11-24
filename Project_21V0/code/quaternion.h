/*
 * quaternion.h
 *
 *  Created on: 2025年6月12日
 *      Author: 吉祥止止
 */

#ifndef CODE_QUATERNION_H_
#define CODE_QUATERNION_H_

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

typedef struct{
    double Xdata;//零飘参数X
    double Ydata;//零飘参数Y
    double Zdata;//零飘参数Z
}gyro_param_t;


extern double Roll,Pitch,Yaw;
extern double yaw;

extern gyro_param_t Gyro_Offset;

void quaternion_init(void);
void quaternion_update(void);
const quaternion_t* get_euler_angles(void);
void Get_Angle(void);


#endif /* CODE_QUATERNION_H_ */
