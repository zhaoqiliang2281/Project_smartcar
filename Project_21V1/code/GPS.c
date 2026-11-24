/*
 * GPS.c
 *
 *  Created on: 2025年6月15日
 *      Author: LiangZhaoqi
 */

#include "zf_common_headfile.h"



double LAT_data_go[Point_MAX] = {0};  //纬度存储数组
double LOG_data_go[Point_MAX] = {0};  //经度存储数组

double azimuth = 0.0;   //方位角
double angle_Error = 0; // 计算当前航向与目标角度之差（角度制），用于控制车辆转向GPS目标点

double Distance = 0;    //GPS当前位置与目标点之间的距离

/************************************************
 *  函数名称: GPS初始化
 *  函数说明:
 *  输入说明:
 *  输   出:
 *  返回值:
 *  更新时间: 2025年6月15日
 *  注   意:
 ************************************************/
void GPS_init(void)
{
    gnss_init(GN42A);
}

/************************************************
 *  函数名称: GPS解析
 *  函数说明: 读取GPS的当前数据
 *  输入说明:
 *  输   出:
 *  返回值:
 *  更新时间: 2025年6月15日
 *  注   意:
 ************************************************/
void GPS_parse(void)
{
    if(gnss_flag)
    {
        gnss_flag = 0;
        gnss_data_parse();           //开始数据解析
    }
}

/************************************************
 *  函数名称: GPS显示
 *  函数说明: 显示GPS信息的时间、位置、速度等
 *  输入说明:
 *  输   出:
 *  返回值:
 *  更新时间: 2025年6月15日
 *  注   意:
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
 *  函数名称: 计算两个角度最小差值
 *  函数说明: 计算两个角度之间的最小角度差（基于360度坐标系）
 *  输入说明:
 *  输   出: 返回角度差值
 *  返回值: 角度差值
 *  更新时间: 2025年6月16日
 *  注   意:
 ************************************************/
double angle_diff(double a, double b)//计算两个角度最小角度差（基于360度坐标系）
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

//*************************GPS漂移修正***********************************

double Gnss_Lat_Correct = 0;    // GPS纬度漂移
double Gnss_Lon_Correct = 0;    // GPS经度漂移
void GPS_Remote(void)//gps漂移修正
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
 *  函数名称: gps距离计算
 *  函数说明: 获取每个点位距离值，并存入home_run[L]数组中
 *  输入说明:
 *  输   出:
 *  返回值:
 *  更新时间: 2025年6月16日
 *  注   意:
 ************************************************/
 void get_point_distance(void)
 {
     int L = 0;
     for(L = 0;L<=17;L++)
     {
         home_run[L] = get_two_points_distance(gnss.latitude, gnss.longitude, LAT_data_go[L], LOG_data_go[L]);
     }
 }

/************************************************
  *  函数名称: flash读取点位转换
  *  函数说明: 将LAT_data1[L]中的值缩小后赋值给LAT_data_go[L]
  *  输入说明:
  *  输   出:
  *  返回值:
  *  更新时间: 2025年6月16日
  *  注   意:
  ************************************************/
/* KM2: lat/lon -> XY (origin=waypoint[0], meters) */
float KM2_GPS_X[16] = {0};
float KM2_GPS_Y[16] = {0};

void km2_de2xy(void)
{
    double lat0 = LAT_data_go[0] * 3.1415926535 / 180.0;
    double cos_lat0 = cos(lat0);
    for(int i = 0; i < 16; i++)
    {
        KM2_GPS_X[i] = (float)((LOG_data_go[i] - LOG_data_go[0]) * 111320.0 * cos_lat0);
        KM2_GPS_Y[i] = (float)((LAT_data_go[i] - LAT_data_go[0]) * 111320.0);
    }
}

void km2_xy2de(void)
{
    double lat0 = LAT_data_go[0] * 3.1415926535 / 180.0;
    double cos_lat0 = cos(lat0);
    for(int i = 0; i < 16; i++)
    {
        LOG_data_go[i] = LOG_data_go[0] + (double)KM2_GPS_X[i] / (111320.0 * cos_lat0);
        LAT_data_go[i] = LAT_data_go[0] + (double)KM2_GPS_Y[i] / 111320.0;
    }
}

void Read_flash_point(void)
{
    int L = 0;
    for(L = 0;L<=16;L++)
    {
        LAT_data_go[L] = LAT_data1[L]/1000000.0;    //缩小一百万倍
        LOG_data_go[L] = LOG_data1[L]/1000000.0;
    }
    km2_de2xy();

}

/************************************************
  *  函数名称: 转换gps数据存入flash
  *  函数说明: 将LAT_data[L]中的值放大后存入LAT_data1[L]中
  *  输入说明:
  *  输   出:
  *  返回值:
  *  更新时间: 2025年6月16日
  *  注   意: 只能将LAT_data[L]转换存入flash中的LAT_data1[L]，这是为了配合FLASH_Get_KMX(void)函数中读取flash后调用
  ************************************************/
void Up_flash_point(void)
{
    int L = 0;
    for(L = 0;L<=16;L++)
    {
        LAT_data1[L] = LAT_data[L]*1000000.0;   //放大一百万倍
        LOG_data1[L] = LOG_data[L]*1000000.0;
    }
}

/************************************************
  *  函数名称: 转换gps数据存入flash
  *  函数说明: 将LAT_data_go中的数据放大后存入LAT_data1[L]中,再统一存入fLash
  *  输入说明:
  *  输   出:
  *  返回值:
  *  更新时间: 2025年6月16日
  *  注   意: 只能将LAT_data_go[L]转换存入flash中的LAT_data1[L]，这是为了配合FLASH_Get_KMX(void)函数中读取flash后调用
  ************************************************/
void Up_flash_point_2(void)
{
    int L = 0;
    for(L = 0;L<=16;L++)
    {
        LAT_data1[L] = (uint32)(LAT_data_go[L]*1000000.0 + 0.5);   // +0.5 四舍五入，避免浮点截断误差
        LOG_data1[L] = (uint32)(LOG_data_go[L]*1000000.0 + 0.5);
    }
}

/************************************************
  *  函数名称: 计算方位角度
  *  函数说明: 计算当前位置与目标点之间的方位角
  *  输入说明:
  *  输   出:
  *  返回值:
  *  更新时间: 2025年6月16日
  *  注   意: GPS运行时,azimuth为GPS方位角,angle_Error为航向与GPS方位角偏差
  ************************************************/
void GPS_Two_point_error(void)
{
    //计算方位角度
    azimuth = get_two_points_azimuth(gnss.latitude, gnss.longitude, LAT_data_go[next], LOG_data_go[next]);
    angle_Error = angle_diff(azimuth,Yaw);
}

/************************************************
  *  函数名称: 计算两点距离
  *  函数说明: 计算当前位置与目标点之间的距离
  *  输入说明:
  *  输   出:
  *  返回值:
  *  更新时间: 2025年6月26日
  *  注   意:
  ************************************************/
void GPS_Two_point_Distance(void)
{
    //计算两点距离
    Distance = get_two_points_distance(gnss.latitude, gnss.longitude, LAT_data_go[next], LOG_data_go[next]);
}
