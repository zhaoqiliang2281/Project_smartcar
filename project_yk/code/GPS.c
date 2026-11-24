/*
 * GPS.c
 *
 *  Created on: 2025年6月15日
 *      Author: 吉祥止止
 */

#include "zf_common_headfile.h"

double Gnss_Lat_Correct = 0;    // GPS除零漂
double Gnss_Lon_Correct = 0;    // GPS除零漂

double LAT_data_go[Point_MAX] = {0};  //纬度存储数组
double LOG_data_go[Point_MAX] = {0};  //经度存储数组

double azimuth = 0.0;   //方位角
double angle_Error = 0; // 方向角与航向角之间的误差（用于控制）计算陀螺仪与GPS航线角误差

double Distance = 0;    //GPS计算出两点的距离误差

/************************************************
 *  函数名称: GPS初始化：GPS_init(void)
 *  功能说明：
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月15日
 *  备   注：
 ************************************************/
void GPS_init(void)
{
    gnss_init(GN42A);
}

/************************************************
 *  函数名称: GPS解析：GPS_parse(void)
 *  功能说明：读取GPS数据前调用
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月15日
 *  备   注：
 ************************************************/
void GPS_parse(void)
{
    if(gnss_flag)
    {
        gnss_flag = 0;
        gnss_data_parse();           //开始解析数据
    }
}

/************************************************
 *  函数名称: GPS显示：GPS_show(void)
 *  功能说明：显示GPS收到的各个数据变量
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月15日
 *  备   注：
 ************************************************/
void GPS_show(void)
{
    ips200_show_uint(   0, 16*0, gnss.time.year, 4);
    ips200_show_uint(  80, 16*0, gnss.time.month, 2);
    ips200_show_uint( 160, 16*0, gnss.time.day, 2);
    ips200_show_uint(   0, 16*1, gnss.time.hour, 2);
    ips200_show_uint(  80, 16*1, gnss.time.minute, 2);
    ips200_show_uint( 160, 16*1, gnss.time.second, 2);

    ips200_show_uint(   0, 16*2, gnss.state, 5);
    ips200_show_float(120, 16*2, gnss.latitude, 4, 6);
    ips200_show_float(  0, 16*3, gnss.longitude, 4, 6);
    ips200_show_float(120, 16*3, gnss.speed, 4, 6);
    ips200_show_float(  0, 16*4, gnss.direction, 4, 6);
    ips200_show_uint( 120, 16*4, gnss.satellite_used, 5);
    ips200_show_float(  0, 16*5, gnss.height, 4, 6);
}

/************************************************
 *  函数名称: 计算两角最小角度差：angle_diff(double a, double b)
 *  功能说明：计算两角最小角度差（均360坐标系）
 *  参数说明：
 *  参   数：陀螺仪角度与GPS航向角
 *  函数返回：角度误差
 *  修改时间：2025年6月16日
 *  备   注：
 ************************************************/
double angle_diff(double a, double b)//计算两角最小角度差（均360坐标系）
{
    double d1, d2;
    d1 = a-b;
    d2 = 360 - fabs(d1);
    if(d1 > 0)
    d2 *= -1.0;
    if(fabs(d1) < fabs(d2))
    return(d1);
    else
    return(d2);
}

//*************************GPS除零漂***********************************
/**/
//double Gnss_Lat_Correct = 0;    // GPS除零漂
//double Gnss_Lon_Correct = 0;    // GPS除零漂
void GPS_Remote(void)//gps除零漂
{
    if(get_two_points_distance(gnss.latitude, gnss.longitude, LAT_data_go[0],LOG_data_go[0])<2)
    {
        Gnss_Lat_Correct=gnss.latitude-LAT_data_go[0];
        Gnss_Lon_Correct=gnss.longitude-LOG_data_go[0];
        for(int j =0; j<=10;j++)
        {
            LAT_data_go[j]+=Gnss_Lat_Correct;
            LOG_data_go[j]+=Gnss_Lon_Correct;
        }
    }
}


/************************************************
 *  函数名称: gps的点位距离：get_point_distance(void)
 *  功能说明：获取每个点位的误差，存放在home_run[L]数组中
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月16日
 *  备   注：
 ************************************************/
 void get_point_distance(void)
 {
     int L = 0;
     for(L = 0;L<=16;L++)
     {
         home_run[L] = get_two_points_distance(gnss.latitude, gnss.longitude, LAT_data_go[L], LOG_data_go[L]);
     }
 }

/************************************************
  *  函数名称: flash中读出点位的转换：Read_flash_point(void)
  *  功能说明：将LAT_data1[L]中的数据小数化读出放在LAT_data_go[L]数组中
  *  参数说明：
  *  参   数：无
  *  函数返回：无
  *  修改时间：2025年6月16日
  *  备   注：
  ************************************************/
void Read_flash_point(void)
{
    int L = 0;
    for(L = 0;L<=16;L++)
    {
        LAT_data_go[L] = LAT_data1[L]/1000000.0;    //整形小数化
        LOG_data_go[L] = LOG_data1[L]/1000000.0;
    }
}

/************************************************
  *  函数名称: 转换gps的类型，存入flash：Read_flash_point(void)
  *  功能说明：将LAT_data[L]中的数据整形化存放在LAT_data1[L]数组中
  *  参数说明：
  *  参   数：无
  *  函数返回：无
  *  修改时间：2025年6月16日
  *  备   注：只是将LAT_data[L]转入存进flash的数组LAT_data1[L]，还需调用FLASH_Get_KMX(void)存进对应科目的flash中
  ************************************************/
void Up_flash_point(void)
{
    int L = 0;
    for(L = 0;L<=16;L++)
    {
        LAT_data1[L] = LAT_data[L]*1000000.0;   //小数整形化
        LOG_data1[L] = LOG_data[L]*1000000.0;
    }
}

/************************************************
  *  函数名称: 计算方向误差：GPS_Two_point_error(void)
  *  功能说明：计算两点方向误差，用于舵机控制
  *  参数说明：
  *  参   数：无
  *  函数返回：无
  *  修改时间：2025年6月16日
  *  备   注：GPS已解析时,azimuth：GPS航向角，angle_Error：陀螺仪与GPS航向角误差
  ************************************************/
void GPS_Two_point_error(void)
{
    //计算两点方向误差
    azimuth = get_two_points_azimuth(gnss.latitude, gnss.longitude, LAT_data_go[next], LOG_data_go[next]);
    angle_Error = angle_diff(azimuth,Yaw);
}

/************************************************
  *  函数名称: 计算两点距离：GPS_Two_point_Distance(void)
  *  功能说明：计算两点距离
  *  参数说明：
  *  参   数：无
  *  函数返回：无
  *  修改时间：2025年6月26日
  *  备   注：
  ************************************************/
void GPS_Two_point_Distance(void)
{
    //计算两点距离
    Distance = get_two_points_distance(gnss.latitude, gnss.longitude, LAT_data_go[next], LOG_data_go[next]);
}


