/*
 * GPS.h
 *
 *  Created on: 2025年6月15日
 *      Author: LiangZhaoqi
 */

#ifndef CODE_GPS_H_
#define CODE_GPS_H_


extern int32  TIM_0_flag;
extern double LAT_data[Point_MAX];  //纬度存储数组
extern double LOG_data[Point_MAX];  //经度存储数组


extern double LAT_data_go[Point_MAX];  //纬度存储数组
extern double LOG_data_go[Point_MAX];  //经度存储数组

extern double GPS_data[4][Point_MAX];

extern double azimuth;      //方位角
extern double angle_Error;  // 方向角与航向角之间的误差（用于控制）

extern double Distance;     //GPS计算出两点的距离误差

void GPS_init(void);
void GPS_parse(void);
void GPS_Remote(void);//gps除零漂
void GPS_show(void);
void GPS_array(void);
double angle_diff(double a, double b);

void Read_flash_point(void);
void get_point_distance(void);
void Up_flash_point(void);
void Up_flash_point_2(void);

void GPS_Two_point_error(void);
void GPS_Two_point_Distance(void);

#endif /* CODE_GPS_H_ */
