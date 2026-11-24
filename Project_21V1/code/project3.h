/*
 * project3.h
 *
 *  Created on: 2026年5月26日
 *      Author: 耿子卓
 */

#ifndef CODE_PROJECT3_H_
#define CODE_PROJECT3_H_

#define GPS_Set_mileage 2400   // 里程计间隔（560≈20cm）

#define P3GPS_MAX 249

extern int32 P3_GPS_Read_go_num;
extern int32 P3_GPS_Read_Back_num;

void System_NSetting_KM3(void);
void Track_Km3(void);

void System_gps_KM3(void);
void GPS_Km3(void);

#endif /* CODE_PROJECT3_H_ */
