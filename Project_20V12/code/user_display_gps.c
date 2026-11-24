/*
 * user_display_gps.c
 *
 *  Created on: 2025年6月16日
 *      Author: LiangZhaoqi
 */

#include "zf_common_headfile.h"

static int16 display_start_x     = 0;
static int16 display_start_y     = 0;
static int16 display_width       =0;
static int16 display_high =0;
static int16 transition_point_num =0;

static plane_point plane_point_data[DISPLAY_POINT_MAX];
static screen_point screen_point_data[DISPLAY_POINT_MAX];

static screen_type_enum display_screen_type = IPS200_TYPE_SPI;
gps_point gps_point_test[10];

//  函数简介  经纬度  球面坐标系  转换为 平面坐标系  单位为米
//  参数说明  gps_point_input 经纬度数据
static void spherical_to_plane(gps_point *gps_point_input)
{
    float meters_per_degree_lat = 0;
    float meters_per_degree_lon = 0;
    if(transition_point_num >1)
    {
        meters_per_degree_lat = (USER_PI/180.0)*EARTH_RADIUS1;   //计算纬度每度米数
        meters_per_degree_lon = meters_per_degree_lat * cosf(gps_point_input[0].lat * USER_PI / 180.0); //计算经度每度米数

        for(int i= 1;i<transition_point_num;i++)
        {
            plane_point_data[i].x = (gps_point_input[i].lon - gps_point_input[0].lon) * meters_per_degree_lon;   //计算平面坐标，东方向位移

            plane_point_data[i].y = (gps_point_input[i].lat - gps_point_input[0].lat) * meters_per_degree_lat;   //计算平面坐标，北方向位移
        }

    }

}
//函数简介  平面坐标系  转换为  屏幕坐标系
static void plane_to_screen(void)
{
    double min_x = plane_point_data[0].x;
    double max_x = plane_point_data[0].x;
    double min_y = plane_point_data[0].y;
    double max_y = plane_point_data[0].y;

    double width   =  0.0f;
    double height  =  0.0f;
    double scale_x =  0.0f;
    double scale_y =  0.0f;
    double scale   =  0.0f;

    double x_offset = 0.0;
    double y_offset = 0.0;

    if(transition_point_num >1)
    {
        for(int i=1;i<transition_point_num;i++)   //找出坐标范围
        {
            min_x = fminf(min_x,plane_point_data[i].x);
            max_x = fmaxf(max_x,plane_point_data[i].x);
            min_y = fminf(min_y,plane_point_data[i].y);
            max_y = fmaxf(max_y,plane_point_data[i].y);
        }

        if(max_x - min_x >EPSILON && max_y - min_y >EPSILON)  //所有点位没有重合 则进行下一步计算
        {
            //计算缩放比例
            width = max_x-min_x;  //坐标范围  宽度

            height = max_y - min_y;  //坐标范围  高度

            scale_x = (float)display_width / width; //计算宽度缩放比例

            scale_y =  (float)display_high / height; //计算高度缩放比例

            scale = fminf(scale_x,scale_y);    //以更小的缩放比作为最终缩放比

            if(scale == scale_x)   //宽度填满，高度居中
            {
                y_offset = ((float)display_high - height * scale) / 2.0;
            }
            else
            {
                x_offset = ((float)display_width - width *scale) / 2.0;
            }

            for (int i= 0;i<transition_point_num;i++)
            {
                screen_point_data[i].x = (plane_point_data[i].x - min_x) * scale + x_offset;
                // 北方向对应屏幕的上方
                // 平面坐标系：y 越大表示越北
                // 屏幕坐标系：y 越大表示越南
                screen_point_data[i].y = (float)display_high - ((plane_point_data[i].y-min_y) * scale + y_offset);
            }
        }
    }

}
//函数简介  GPS坐标系  转换为  屏幕显示位置
//参数说明  gps_point_input  经纬度数据
//参数说明  point_num        数据长度
void user_gps_transition(gps_point *gps_point_input,int16 point_num)
{
    zf_assert(point_num < DISPLAY_POINT_MAX);   //需要显示的点的数量超过了最大显示数量  需要自行修改  DISPLAY_POINT_MAX的值

    transition_point_num  = point_num;

    memset(plane_point_data,0,sizeof(plane_point_data));  //清除之前的平面坐标系数据
    //memset 将一段内存空间设置为指定的值
    memset(screen_point_data , 0,sizeof(screen_point_data));  //清除之前的屏幕坐标数据

    spherical_to_plane(gps_point_input);    //经纬度球面坐标系  转化为  平面坐标系

    plane_to_screen();                       //平面坐标系  转换为  屏幕显示点位

}

//函数简介  屏幕显示输出函数
//参数说明  display_color 绘制的边线颜色（可通过将颜色绘制为背景的方式实现  擦除）
void user_gps_display(uint16 display_color)
{
    for(int i =0 ;i<(transition_point_num - 1);i++)
    {
        ips200_draw_line(                        (uint16)screen_point_data[i].x  + display_start_x,
                                                 (uint16)screen_point_data[i].y  + display_start_y,
                                                 (uint16)screen_point_data[i+1].x + display_start_x,
                                                 (uint16)screen_point_data[i+1].y + display_start_y,
                                                 display_color);
    }
}
// 函数简介  GPS坐标显示  初始化
// 参数说明  screen_type  屏幕类型  参考 screen_type_enum 枚举
// 参数说明  start_x  显示范围起点x
// 参数说明  start_y  显示范围起点y
// 参数说明  width    显示宽度
// 参数说明  high     显示高度
void user_gps_display_init(screen_type_enum screen_type,int16 start_x,int16 start_y,int16 width,int16 high)
{
    display_start_x   = start_x;
    display_start_y   = start_y;
    display_width     = width - 1;
    display_high      = high  - 1;
    display_screen_type= screen_type;
}
void Function(void)
{
    user_gps_display_init(SCREEN_IPS200_SPI,10,10,220,300);
    user_gps_transition(gps_point_test,4);
    user_gps_display(RGB565_RED);
}


