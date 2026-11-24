/*
 * path_diagram.c
 *
 *  Created on: 2025年6月9日
 *      Author: liuliangfang
 */

#include "zf_common_headfile.h"


void path_image(void)
{
    if(key1_flag == 1)   //查看路径图（科目一）
               {
                   key1_flag=0;
                   next = 0;
                   ips200_clear();

                             gps_point gps_point_test[]=
                             {
                                 {LAT_data_go[0],LOG_data_go[0]},
                                 {LAT_data_go[1],LOG_data_go[1]},
                                 {LAT_data_go[2],LOG_data_go[2]},
                                 {LAT_data_go[3],LOG_data_go[3]},
                                 {LAT_data_go[4],LOG_data_go[4]},
                                 {LAT_data_go[5],LOG_data_go[5]},
                                 {LAT_data_go[6],LOG_data_go[6]},
                                 {LAT_data_go[7],LOG_data_go[7]},
                                 {LAT_data_go[8],LOG_data_go[8]},

                                 {LAT_data_go[9],LOG_data_go[9]},
                                 {LAT_data_go[10],LOG_data_go[10]},

                             };
                             user_gps_display_init(IPS200_TYPE_SPI,10,10,220,300);
                             user_gps_transition(gps_point_test,sizeof(gps_point_test) / sizeof(gps_point_test[0]));
                             user_gps_display(RGB565_PURPLE);

                   Buzzer_check(50);
               }

    if(key2_flag == 1)//查看路径图（科目二）
                  {
                      key2_flag=0;
                      next = 0;
                      ips200_clear();

                                gps_point gps_point_test[]=
                                {
                                    {LAT_data_go[0],LOG_data_go[0]},
                                    {LAT_data_go[1],LOG_data_go[1]},
                                    {LAT_data_go[2],LOG_data_go[2]},
                                    {LAT_data_go[3],LOG_data_go[3]},
                                    {LAT_data_go[4],LOG_data_go[4]},
                                    {LAT_data_go[5],LOG_data_go[5]},
                                    {LAT_data_go[6],LOG_data_go[6]},
                                    {LAT_data_go[7],LOG_data_go[7]},
                                    {LAT_data_go[8],LOG_data_go[8]},
                                    {LAT_data_go[9],LOG_data_go[9]},
                                    {LAT_data_go[10],LOG_data_go[10]},

                                };
                                user_gps_display_init(IPS200_TYPE_SPI,10,10,220,300);
                                user_gps_transition(gps_point_test,sizeof(gps_point_test) / sizeof(gps_point_test[0]));
                                user_gps_display(RGB565_YELLOW);
                      Buzzer_check(50);
                  }

    if(key3_flag == 1)   //查看路径图
                    {
                        key3_flag=0;
                        next = 0;


                        ips200_clear();

                                    gps_point gps_point_test[]=
                                 {
                                     {LAT_data_go[0],LOG_data_go[0]},
                                     {LAT_data_go[1],LOG_data_go[1]},
                                     {LAT_data_go[2],LOG_data_go[2]},
                                     {LAT_data_go[3],LOG_data_go[3]},
                                     {LAT_data_go[4],LOG_data_go[4]},
                                     {LAT_data_go[5],LOG_data_go[5]},
                                     {LAT_data_go[6],LOG_data_go[6]},
                                     {LAT_data_go[7],LOG_data_go[7]},
                                     {LAT_data_go[8],LOG_data_go[8]},
                                     {LAT_data_go[9],LOG_data_go[9]},
                                     {LAT_data_go[10],LOG_data_go[10]},
                                     {LAT_data_go[11],LOG_data_go[11]},
                                     {LAT_data_go[12],LOG_data_go[12]},

                                 };
                                  user_gps_display_init(IPS200_TYPE_SPI,10,10,220,300);
                                  user_gps_transition(gps_point_test,sizeof(gps_point_test) / sizeof(gps_point_test[0]));
                                  user_gps_display(RGB565_GREEN);

                        Buzzer_check(50);
                    }

}

//***********************************************************************

int16 GL_NUM=0;//采集当前点数（存入）
int N = 0;
int Num_flag = 0; //用于按键修改经纬度的标志位
double r = 0.000003;

void Key_Change_LAT_LON(void)
{

if(flag_go == 1)
{
    //科目一
    if(key1_flag)
        {
            N=0;  Num_flag =0; ips200_clear();
            while(key1_flag)
            {
                if(key2_flag)
                {
                    if(Num_flag ==0)  {N++;}
                    if(Num_flag ==1)  {N--;}
                    if(N == 7)  {Num_flag =1;}
                    if(N == 0)  {Num_flag =0;}
                    key2_flag =0;       ips200_show_int(1, 16*3, Num_flag, 3);
                }
                if(key3_flag)   {LAT_data_go[N] +=r;  key3_flag = 0;}
                if(key4_flag)   {LAT_data_go[N] -=r;  key4_flag = 0;}
                if(gpio_get_level(sw2)) { key1_flag = 0;}
                ips200_show_string(0, 16*1, "N");           ips200_show_int(100, 16*1, N, 2);
                ips200_show_string(0, 16*2, "LAT");         ips200_show_float(100, 16*2,LAT_data_go[N], 3, 6);

                if(key5_flag)
                {
                    key5_flag=0;
                    ips200_clear();
                }

                gps_point gps_point_test[]=
                {
                    {LAT_data_go[0],LOG_data_go[0]},
                    {LAT_data_go[1],LOG_data_go[1]},
                    {LAT_data_go[2],LOG_data_go[2]},
                    {LAT_data_go[3],LOG_data_go[3]},
                    {LAT_data_go[4],LOG_data_go[4]},
                    {LAT_data_go[5],LOG_data_go[5]},
                    {LAT_data_go[6],LOG_data_go[6]},
                    {LAT_data_go[7],LOG_data_go[7]},

                };
                user_gps_display_init(IPS200_TYPE_SPI,10,10,220,300);
                user_gps_transition(gps_point_test,sizeof(gps_point_test) / sizeof(gps_point_test[0]));
                user_gps_display(RGB565_BLUE);

                ips200_show_float(8*3,16*5,gnss.latitude-LAT_data_go[N],4,6);
                ips200_show_float(8*3,16*6,gnss.longitude-LOG_data_go[N],4,6);

                ips200_show_float(8*3,16*3,gnss.latitude,4,6);
                ips200_show_float(8*3,16*4,gnss.longitude,4,6);
            }
        }

        if(key2_flag)
        {
            N=0;    Num_flag =0;    ips200_clear();
            while(key2_flag)
            {
                if(key1_flag)
                {
                    if(Num_flag ==0)  {N++;}
                    if(Num_flag ==1)  {N--;}
                    if(N == 7)  {Num_flag =1;}
                    if(N == 0)  {Num_flag =0;}
                    key1_flag =0;   ips200_show_int(1, 16*3, Num_flag, 3);
                }
                if(key3_flag)   {LOG_data_go[N] +=r;  key3_flag = 0;}
                if(key4_flag)   {LOG_data_go[N] -=r;  key4_flag = 0;}
                if(gpio_get_level(sw2))               { key2_flag = 0;}
                ips200_show_string(0, 16*1, "N");           ips200_show_int(100, 16*1, N, 2);
                ips200_show_string(0, 16*2, "LOG");         ips200_show_float(100, 16*2,LOG_data_go[N], 3, 6);
                if(key5_flag)
                {
                    key5_flag=0;
                    ips200_clear();
                }


                gps_point gps_point_test[]=
                {
                        {LAT_data_go[0],LOG_data_go[0]},
                        {LAT_data_go[1],LOG_data_go[1]},
                        {LAT_data_go[2],LOG_data_go[2]},
                        {LAT_data_go[3],LOG_data_go[3]},
                        {LAT_data_go[4],LOG_data_go[4]},
                        {LAT_data_go[5],LOG_data_go[5]},
                        {LAT_data_go[6],LOG_data_go[6]},
                        {LAT_data_go[7],LOG_data_go[7]},

                };
                user_gps_display_init(IPS200_TYPE_SPI,10,10,220,300);
                user_gps_transition(gps_point_test,sizeof(gps_point_test) / sizeof(gps_point_test[0]));
                user_gps_display(RGB565_RED);

                ips200_show_float(8*3,16*5,gnss.latitude-LAT_data_go[N],4,6);
                ips200_show_float(8*3,16*6,gnss.longitude-LOG_data_go[N],4,6);

                ips200_show_float(8*3,16*3,gnss.latitude,4,6);
                ips200_show_float(8*3,16*4,gnss.longitude,4,6);
            }
        }
}
 /////   //科目二////////////////

if(flag_go == 2)
{
    if(key1_flag)
    {
        N=0;  Num_flag =0; ips200_clear();
        while(key1_flag)
        {
            if(key2_flag)
            {
                if(Num_flag ==0)  {N++;}
                if(Num_flag ==1)  {N--;}
                if(N == 14)  {Num_flag =1;}
                if(N == 0)  {Num_flag =0;}
                key2_flag =0;       ips200_show_int(1, 16*3, Num_flag, 3);
            }
            if(key3_flag)   {LAT_data_go[N] +=r;  key3_flag = 0;}
            if(key4_flag)   {LAT_data_go[N] -=r;  key4_flag = 0;}
            if(gpio_get_level(sw2)) { key1_flag = 0;}
            ips200_show_string(0, 16*1, "N");           ips200_show_int(100, 16*1, N, 2);
            ips200_show_string(0, 16*2, "LAT");         ips200_show_float(100, 16*2,LAT_data_go[N], 3, 6);

            if(key5_flag)
            {
                key5_flag=0;
                ips200_clear();
            }

            gps_point gps_point_test[]=
            {
                {LAT_data_go[0],LOG_data_go[0]},
                {LAT_data_go[1],LOG_data_go[1]},
                {LAT_data_go[2],LOG_data_go[2]},
                {LAT_data_go[3],LOG_data_go[3]},
                {LAT_data_go[4],LOG_data_go[4]},
                {LAT_data_go[5],LOG_data_go[5]},
                {LAT_data_go[6],LOG_data_go[6]},
                {LAT_data_go[7],LOG_data_go[7]},
                {LAT_data_go[8],LOG_data_go[8]},
                {LAT_data_go[9],LOG_data_go[9]},
                {LAT_data_go[10],LOG_data_go[10]},
                {LAT_data_go[11],LOG_data_go[11]},
                {LAT_data_go[12],LOG_data_go[12]},
                {LAT_data_go[13],LOG_data_go[13]},
            };
            user_gps_display_init(IPS200_TYPE_SPI,10,10,220,300);
            user_gps_transition(gps_point_test,sizeof(gps_point_test) / sizeof(gps_point_test[0]));
            user_gps_display(RGB565_BLUE);

            ips200_show_float(8*3,16*5,gnss.latitude-LAT_data_go[N],4,6);
            ips200_show_float(8*3,16*6,gnss.longitude-LOG_data_go[N],4,6);

            ips200_show_float(8*3,16*3,gnss.latitude,4,6);
            ips200_show_float(8*3,16*4,gnss.longitude,4,6);
        }
    }

    if(key2_flag)
    {
        N=0;    Num_flag =0;    ips200_clear();
        while(key2_flag)
        {
            if(key1_flag)
            {
                if(Num_flag ==0)  {N++;}
                if(Num_flag ==1)  {N--;}
                if(N == 14)  {Num_flag =1;}
                if(N == 0)  {Num_flag =0;}
                key1_flag =0;   ips200_show_int(1, 16*3, Num_flag, 3);
            }
            if(key3_flag)   {LOG_data_go[N] +=r;  key3_flag = 0;}
            if(key4_flag)   {LOG_data_go[N] -=r;  key4_flag = 0;}
            if(gpio_get_level(sw2))               { key2_flag = 0;}
            ips200_show_string(0, 16*1, "N");           ips200_show_int(100, 16*1, N, 2);
            ips200_show_string(0, 16*2, "LOG");         ips200_show_float(100, 16*2,LOG_data_go[N], 3, 6);
            if(key5_flag)
            {
                key5_flag=0;
                ips200_clear();
            }


            gps_point gps_point_test[]=
            {
                    {LAT_data_go[0],LOG_data_go[0]},
                    {LAT_data_go[1],LOG_data_go[1]},
                    {LAT_data_go[2],LOG_data_go[2]},
                    {LAT_data_go[3],LOG_data_go[3]},
                    {LAT_data_go[4],LOG_data_go[4]},
                    {LAT_data_go[5],LOG_data_go[5]},
                    {LAT_data_go[6],LOG_data_go[6]},
                    {LAT_data_go[7],LOG_data_go[7]},
                    {LAT_data_go[8],LOG_data_go[8]},
                    {LAT_data_go[9],LOG_data_go[9]},
                    {LAT_data_go[10],LOG_data_go[10]},
                    {LAT_data_go[11],LOG_data_go[11]},
                    {LAT_data_go[12],LOG_data_go[12]},
                    {LAT_data_go[13],LOG_data_go[13]},
            };
            user_gps_display_init(IPS200_TYPE_SPI,10,10,220,300);
            user_gps_transition(gps_point_test,sizeof(gps_point_test) / sizeof(gps_point_test[0]));
            user_gps_display(RGB565_RED);

            ips200_show_float(8*3,16*5,gnss.latitude-LAT_data_go[N],4,6);
            ips200_show_float(8*3,16*6,gnss.longitude-LOG_data_go[N],4,6);

            ips200_show_float(8*3,16*3,gnss.latitude,4,6);
            ips200_show_float(8*3,16*4,gnss.longitude,4,6);
        }
    }
}


if(flag_go == 3)
{
    if(key1_flag)
    {
        N=0;  Num_flag =0; ips200_clear();
        while(key1_flag)
        {
            if(key2_flag)
            {
                if(Num_flag ==0)  {N++;}
                if(Num_flag ==1)  {N--;}
                if(N == 14)  {Num_flag =1;}
                if(N == 0)  {Num_flag =0;}
                key2_flag =0;       ips200_show_int(1, 16*3, Num_flag, 3);
            }
            if(key3_flag)   {LAT_data_go[N] +=r;  key3_flag = 0;}
            if(key4_flag)   {LAT_data_go[N] -=r;  key4_flag = 0;}
            if(gpio_get_level(sw2)) { key1_flag = 0;}
            ips200_show_string(0, 16*1, "N");           ips200_show_int(100, 16*1, N, 2);
            ips200_show_string(0, 16*2, "LAT");         ips200_show_float(100, 16*2,LAT_data_go[N], 3, 6);

            if(key5_flag)
            {
                key5_flag=0;
                ips200_clear();
            }

            gps_point gps_point_test[]=
            {
                {LAT_data_go[0],LOG_data_go[0]},
                {LAT_data_go[1],LOG_data_go[1]},
                {LAT_data_go[2],LOG_data_go[2]},
                {LAT_data_go[3],LOG_data_go[3]},
                {LAT_data_go[4],LOG_data_go[4]},
                {LAT_data_go[5],LOG_data_go[5]},
                {LAT_data_go[6],LOG_data_go[6]},
                {LAT_data_go[7],LOG_data_go[7]},
                {LAT_data_go[8],LOG_data_go[8]},
                {LAT_data_go[9],LOG_data_go[9]},
                {LAT_data_go[10],LOG_data_go[10]},
                {LAT_data_go[11],LOG_data_go[11]},
                {LAT_data_go[12],LOG_data_go[12]},
                {LAT_data_go[13],LOG_data_go[13]},
            };
            user_gps_display_init(IPS200_TYPE_SPI,10,10,220,300);
            user_gps_transition(gps_point_test,sizeof(gps_point_test) / sizeof(gps_point_test[0]));
            user_gps_display(RGB565_BLUE);

            ips200_show_float(8*3,16*5,gnss.latitude-LAT_data_go[N],4,6);
            ips200_show_float(8*3,16*6,gnss.longitude-LOG_data_go[N],4,6);

            ips200_show_float(8*3,16*3,gnss.latitude,4,6);
            ips200_show_float(8*3,16*4,gnss.longitude,4,6);
        }
    }

    if(key2_flag)
    {
        N=0;    Num_flag =0;    ips200_clear();
        while(key2_flag)
        {
            if(key1_flag)
            {
                if(Num_flag ==0)  {N++;}
                if(Num_flag ==1)  {N--;}
                if(N == 14)  {Num_flag =1;}
                if(N == 0)  {Num_flag =0;}
                key1_flag =0;   ips200_show_int(1, 16*3, Num_flag, 3);
            }
            if(key3_flag)   {LOG_data_go[N] +=r;  key3_flag = 0;}
            if(key4_flag)   {LOG_data_go[N] -=r;  key4_flag = 0;}
            if(gpio_get_level(sw2))               { key2_flag = 0;}
            ips200_show_string(0, 16*1, "N");           ips200_show_int(100, 16*1, N, 2);
            ips200_show_string(0, 16*2, "LOG");         ips200_show_float(100, 16*2,LOG_data_go[N], 3, 6);
            if(key5_flag)
            {
                key5_flag=0;
                ips200_clear();
            }


            gps_point gps_point_test[]=
            {
                    {LAT_data_go[0],LOG_data_go[0]},
                    {LAT_data_go[1],LOG_data_go[1]},
                    {LAT_data_go[2],LOG_data_go[2]},
                    {LAT_data_go[3],LOG_data_go[3]},
                    {LAT_data_go[4],LOG_data_go[4]},
                    {LAT_data_go[5],LOG_data_go[5]},
                    {LAT_data_go[6],LOG_data_go[6]},
                    {LAT_data_go[7],LOG_data_go[7]},
                    {LAT_data_go[8],LOG_data_go[8]},
                    {LAT_data_go[9],LOG_data_go[9]},
                    {LAT_data_go[10],LOG_data_go[10]},
                    {LAT_data_go[11],LOG_data_go[11]},
                    {LAT_data_go[12],LOG_data_go[12]},
                    {LAT_data_go[13],LOG_data_go[13]},
            };
            user_gps_display_init(IPS200_TYPE_SPI,10,10,220,300);
            user_gps_transition(gps_point_test,sizeof(gps_point_test) / sizeof(gps_point_test[0]));
            user_gps_display(RGB565_RED);

            ips200_show_float(8*3,16*5,gnss.latitude-LAT_data_go[N],4,6);
            ips200_show_float(8*3,16*6,gnss.longitude-LOG_data_go[N],4,6);

            ips200_show_float(8*3,16*3,gnss.latitude,4,6);
            ips200_show_float(8*3,16*4,gnss.longitude,4,6);
        }
    }
}

}


//************************************************************
