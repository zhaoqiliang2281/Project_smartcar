/*
 * imu.h
 *
 *  Created on: 2025年6月12日
 *      Author: LiangZhaoqi
 */

#ifndef CODE_IMU_H_
#define CODE_IMU_H_

extern double Daty_Z;                    //Z轴规划,规划为0-180和0-(-180)
extern double T_M;                       //Z轴规划,规划为0-360和0-(-360)(备用1)
extern double Angle;

void IMU_Handle_180(void);              //规划为0-180和0-(-180)   Z_360
void IMU_Handle_360(void);              //规划为0-360和0-(-360)   T_M
double angle_calc(double angle_m, double gyro_m);
double IMU_Get(void);
void get_angle(void);

#endif /* CODE_IMU_H_ */
