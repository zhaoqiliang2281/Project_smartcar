/*
 * user_display_gps.c
 *
 *  Created on: 2025年6月16日
 *      Author: liang2281
 */

#include "zf_common_headfile.h"

static int16 display_start_x     = 0;
static int16 display_start_y     = 0;
static int16 display_width       = 0;
static int16 display_high        = 0;
static int16 transition_point_num = 0;

static plane_point plane_point_data[DISPLAY_POINT_MAX];
static screen_point screen_point_data[DISPLAY_POINT_MAX];

static screen_type_enum display_screen_type = IPS200_TYPE_SPI;
gps_point gps_point_test[10];

//  函数：球面坐标转平面坐标
//  说明：将经纬度坐标系转换为平面坐标系，单位为米
//  参数：gps_point_input 经纬度数据指针
static void spherical_to_plane(gps_point *gps_point_input)
{
    float meters_per_degree_lat = 0;
    float meters_per_degree_lon = 0;
    if(transition_point_num > 1)
    {
        meters_per_degree_lat = (USER_PI/180.0) * EARTH_RADIUS1;   // 每度纬度对应的米数
        meters_per_degree_lon = meters_per_degree_lat * cosf(gps_point_input[0].lat * USER_PI / 180.0); // 每度经度对应的米数

        for(int i = 1; i < transition_point_num; i++)
        {
            plane_point_data[i].x = (gps_point_input[i].lon - gps_point_input[0].lon) * meters_per_degree_lon;   // 计算平面X坐标，单位为米
            plane_point_data[i].y = (gps_point_input[i].lat - gps_point_input[0].lat) * meters_per_degree_lat;   // 计算平面Y坐标，单位为米
        }
    }
}

//  函数：平面坐标转屏幕坐标
//  说明：将平面坐标系转换为屏幕坐标系
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

    if(transition_point_num > 1)
    {
        for(int i = 1; i < transition_point_num; i++)   // 遍历查找坐标范围
        {
            min_x = fminf(min_x, plane_point_data[i].x);
            max_x = fmaxf(max_x, plane_point_data[i].x);
            min_y = fminf(min_y, plane_point_data[i].y);
            max_y = fmaxf(max_y, plane_point_data[i].y);
        }

        if(max_x - min_x > EPSILON && max_y - min_y > EPSILON)  // 确保坐标没有重叠，防止除零错误
        {
            // 计算缩放比例
            width = max_x - min_x;   // 坐标范围宽度
            height = max_y - min_y;  // 坐标范围高度

            scale_x = (float)display_width / width;   // 水平方向缩放比例
            scale_y = (float)display_high / height;   // 垂直方向缩放比例

            scale = fminf(scale_x, scale_y);    // 取较小缩放比例以保证全部显示

            if(scale == scale_x)   // 如果水平比例较小
            {
                y_offset = ((float)display_high - height * scale) / 2.0;
            }
            else
            {
                x_offset = ((float)display_width - width * scale) / 2.0;
            }

            for(int i = 0; i < transition_point_num; i++)
            {
                screen_point_data[i].x = (plane_point_data[i].x - min_x) * scale + x_offset;
                // 注意：平面坐标系中y向上为正
                // 屏幕坐标系中y向下为正
                screen_point_data[i].y = (float)display_high - ((plane_point_data[i].y - min_y) * scale + y_offset);
            }
        }
    }
}

//  函数：GPS坐标系转屏幕显示位置
//  说明：将经纬度坐标转换为屏幕显示坐标
//  参数：gps_point_input  经纬度数据指针
//  参数：point_num        待转换点数量
void user_gps_transition(gps_point *gps_point_input, int16 point_num)
{
    zf_assert(point_num < DISPLAY_POINT_MAX);   // 如果要显示的点数超过最大显示点数，需要修改DISPLAY_POINT_MAX的值

    transition_point_num = point_num;

    memset(plane_point_data, 0, sizeof(plane_point_data));   // 清空平面坐标数据
    memset(screen_point_data, 0, sizeof(screen_point_data)); // 清空屏幕坐标数据

    spherical_to_plane(gps_point_input);    // 经纬度坐标系转换为平面坐标系
    plane_to_screen();                       // 平面坐标系转换为屏幕显示位置
}

//  函数：屏幕显示路径点
//  说明：在屏幕上绘制路径连线
//  参数：display_color 绘制颜色，可通过RGB565颜色宏或自定义颜色值实现
void user_gps_display(uint16 display_color)
{
    for(int i = 0; i < (transition_point_num - 1); i++)
    {
        ips200_draw_line((uint16)screen_point_data[i].x     + display_start_x,
                         (uint16)screen_point_data[i].y     + display_start_y,
                         (uint16)screen_point_data[i+1].x   + display_start_x,
                         (uint16)screen_point_data[i+1].y   + display_start_y,
                         display_color);
    }
}

//  函数：GPS路径显示初始化
//  说明：设置显示区域参数
//  参数：screen_type  屏幕类型，参见screen_type_enum枚举
//  参数：start_x      显示区域起始X
//  参数：start_y      显示区域起始Y
//  参数：width        显示区域宽度
//  参数：high         显示区域高度
void user_gps_display_init(screen_type_enum screen_type, int16 start_x, int16 start_y, int16 width, int16 high)
{
    display_start_x    = start_x;
    display_start_y    = start_y;
    display_width      = width - 1;
    display_high       = high - 1;
    display_screen_type = screen_type;
}

//  函数：获取转换后的屏幕坐标
//  说明：获取指定索引点的屏幕坐标
//  参数：index  点位索引
//  参数：*x     返回屏幕X坐标
//  参数：*y     返回屏幕Y坐标
void user_gps_get_screen_point(int16 index, uint16 *x, uint16 *y)
{
    if(index >= 0 && index < transition_point_num)
    {
        *x = (uint16)screen_point_data[index].x + display_start_x;
        *y = (uint16)screen_point_data[index].y + display_start_y;
    }
    else
    {
        *x = 0;
        *y = 0;
    }
}

//  函数：获取转换后的点数量
//  返回值：int16  当前转换后的点数量
int16 user_gps_get_point_count(void)
{
    return transition_point_num;
}

//  函数：IPS200画圆函数（Bresenham中点画圆算法）
//  说明：使用Bresenham算法在屏幕上绘制圆形
//  参数：cx     圆心X坐标
//  参数：cy     圆心Y坐标
//  参数：r      圆半径
//  参数：color  颜色
void ips200_draw_circle(uint16 cx, uint16 cy, uint16 r, const uint16 color)
{
    int16 x = 0;
    int16 y = r;
    int16 d = 3 - 2 * r;

    while(y >= x)
    {
        ips200_draw_point(cx + x, cy + y, color);
        ips200_draw_point(cx - x, cy + y, color);
        ips200_draw_point(cx + x, cy - y, color);
        ips200_draw_point(cx - x, cy - y, color);
        ips200_draw_point(cx + y, cy + x, color);
        ips200_draw_point(cx - y, cy + x, color);
        ips200_draw_point(cx + y, cy - x, color);
        ips200_draw_point(cx - y, cy - x, color);

        x++;
        if(d > 0)
        {
            y--;
            d = d + 4 * (x - y) + 10;
        }
        else
        {
            d = d + 4 * x + 6;
        }
    }
}

//  函数示例：使用示例
void Function(void)
{
    user_gps_display_init(SCREEN_IPS200_SPI, 10, 10, 220, 300);
    user_gps_transition(gps_point_test, 4);
    user_gps_display(RGB565_RED);
}
