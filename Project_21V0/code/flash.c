/*
 * flash.c
 *
 *  Created on: 2025年6月13日
 *      Author: 吉祥止止
 */

#include "zf_common_headfile.h"


uint32 LAT_data1[Point_MAX] = {0};  //纬度存储数组
uint32 LOG_data1[Point_MAX] = {0};  //经度存储数组

/*========================================科一Flash==========================================*/
/************************************************
 *  函数名称: 将数据存入Flash：FLASH_Get_KM1(void)
 *  功能说明：将数据写入KM1的Flash
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月16日
 *  备   注：
 ************************************************/
void FLASH_Get_KM1(void)
{
    // 清空缓冲区并填充数据
    flash_buffer_clear();

    flash_union_buffer[0].uint32_type =  LAT_data1[0];
    flash_union_buffer[1].uint32_type =  LOG_data1[0];
    flash_union_buffer[2].uint32_type =  LAT_data1[1];
    flash_union_buffer[3].uint32_type =  LOG_data1[1];
    flash_union_buffer[4].uint32_type =  LAT_data1[2];
    flash_union_buffer[5].uint32_type =  LOG_data1[2];
    flash_union_buffer[6].uint32_type =  LAT_data1[3];
    flash_union_buffer[7].uint32_type =  LOG_data1[3];
    flash_union_buffer[8].uint32_type =  LAT_data1[4];
    flash_union_buffer[9].uint32_type =  LOG_data1[4];

    flash_union_buffer[10].uint32_type =  LAT_data1[5];
    flash_union_buffer[11].uint32_type =  LOG_data1[5];
    flash_union_buffer[12].uint32_type =  LAT_data1[6];
    flash_union_buffer[13].uint32_type =  LOG_data1[6];
    flash_union_buffer[14].uint32_type =  LAT_data1[7];
    flash_union_buffer[15].uint32_type =  LOG_data1[7];//8个点

    flash_union_buffer[16].uint32_type =  LAT_data1[8];
    flash_union_buffer[17].uint32_type =  LOG_data1[8];
    flash_union_buffer[18].uint32_type =  LAT_data1[9];
    flash_union_buffer[19].uint32_type =  LOG_data1[9];
    flash_union_buffer[20].uint32_type =  LAT_data1[10];
    flash_union_buffer[21].uint32_type =  LOG_data1[10];
    flash_union_buffer[22].uint32_type =  LAT_data1[11];
    flash_union_buffer[23].uint32_type =  LOG_data1[11];
    flash_union_buffer[24].uint32_type =  LAT_data1[12];
    flash_union_buffer[25].uint32_type =  LOG_data1[12];
    flash_union_buffer[26].uint32_type =  LAT_data1[13];
    flash_union_buffer[27].uint32_type =  LOG_data1[13];//14个点

    flash_union_buffer[28].uint32_type =  LAT_data1[14];
    flash_union_buffer[29].uint32_type =  LOG_data1[14];//15个点


    // 检查并擦除目标页
    if(flash_check(FLASH_SECTION_INDEX, FLASH_GPS_INDEX))
    {
        flash_erase_page(FLASH_SECTION_INDEX, FLASH_GPS_INDEX);
    }

    //将数据写入Flash
    flash_write_page_from_buffer(FLASH_SECTION_INDEX, FLASH_GPS_INDEX);

    ips200_show_string(172, 16*10, "CN flash");
}

/************************************************
 *  函数名称: 将Flash的数据读出：FLASH_READ_KM1(void)
 *  功能说明：将KM1的Flash的数据读出
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月16日
 *  备   注：
 ************************************************/
void FLASH_READ_KM1(void)
{
    //读取操作
    if(flash_check(FLASH_SECTION_INDEX, FLASH_GPS_INDEX))
    {
        flash_read_page_to_buffer(FLASH_SECTION_INDEX, FLASH_GPS_INDEX);

        LAT_data1[0] = flash_union_buffer[0].uint32_type;
        LOG_data1[0] = flash_union_buffer[1].uint32_type;
        LAT_data1[1] = flash_union_buffer[2].uint32_type;
        LOG_data1[1] = flash_union_buffer[3].uint32_type;
        LAT_data1[2] = flash_union_buffer[4].uint32_type;
        LOG_data1[2] = flash_union_buffer[5].uint32_type;
        LAT_data1[3] = flash_union_buffer[6].uint32_type;
        LOG_data1[3] = flash_union_buffer[7].uint32_type;
        LAT_data1[4] = flash_union_buffer[8].uint32_type;
        LOG_data1[4] = flash_union_buffer[9].uint32_type;

        LAT_data1[5] = flash_union_buffer[10].uint32_type;
        LOG_data1[5] = flash_union_buffer[11].uint32_type;
        LAT_data1[6] = flash_union_buffer[12].uint32_type;
        LOG_data1[6] = flash_union_buffer[13].uint32_type;
        LAT_data1[7] = flash_union_buffer[14].uint32_type;
        LOG_data1[7] = flash_union_buffer[15].uint32_type;


        LAT_data1[8] = flash_union_buffer[16].uint32_type;
        LOG_data1[8] = flash_union_buffer[17].uint32_type;
        LAT_data1[9] = flash_union_buffer[18].uint32_type;
        LOG_data1[9] = flash_union_buffer[19].uint32_type;
        LAT_data1[10] = flash_union_buffer[20].uint32_type;
        LOG_data1[10] = flash_union_buffer[21].uint32_type;
        LAT_data1[11] = flash_union_buffer[22].uint32_type;
        LOG_data1[11] = flash_union_buffer[23].uint32_type;
        LAT_data1[12] = flash_union_buffer[24].uint32_type;
        LOG_data1[12] = flash_union_buffer[25].uint32_type;
        LAT_data1[13] = flash_union_buffer[26].uint32_type;
        LOG_data1[13] = flash_union_buffer[27].uint32_type;
        LAT_data1[14] = flash_union_buffer[28].uint32_type;
        LOG_data1[14] = flash_union_buffer[29].uint32_type;

//        for(int i = 0;i<28;i++)
//        {
//            printf("%d\n\r",(int)flash_union_buffer[i].uint32_type);//上位机看值
//        }

        ips200_show_string(172, 16*11, "OK flash");
    }
    else
    {
        ips200_show_string(172, 16*11, "NO flash");
    }
}

/*========================================科二Flash==========================================*/

/************************************************
 *  函数名称: 将数据存入Flash：FLASH_Get_KM2(void)
 *  功能说明：将数据写入KM2的Flash
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月16日
 *  备   注：
 ************************************************/
void FLASH_Get_KM2(void)
{
    // 清空缓冲区并填充数据
    flash_buffer_clear();

    flash_union_buffer[0].uint32_type =  LAT_data1[0];
    flash_union_buffer[1].uint32_type =  LOG_data1[0];
    flash_union_buffer[2].uint32_type =  LAT_data1[1];
    flash_union_buffer[3].uint32_type =  LOG_data1[1];
    flash_union_buffer[4].uint32_type =  LAT_data1[2];
    flash_union_buffer[5].uint32_type =  LOG_data1[2];
    flash_union_buffer[6].uint32_type =  LAT_data1[3];
    flash_union_buffer[7].uint32_type =  LOG_data1[3];
    flash_union_buffer[8].uint32_type =  LAT_data1[4];
    flash_union_buffer[9].uint32_type =  LOG_data1[4];

    flash_union_buffer[10].uint32_type =  LAT_data1[5];
    flash_union_buffer[11].uint32_type =  LOG_data1[5];
    flash_union_buffer[12].uint32_type =  LAT_data1[6];
    flash_union_buffer[13].uint32_type =  LOG_data1[6];
    flash_union_buffer[14].uint32_type =  LAT_data1[7];
    flash_union_buffer[15].uint32_type =  LOG_data1[7];
    flash_union_buffer[16].uint32_type =  LAT_data1[8];
    flash_union_buffer[17].uint32_type =  LOG_data1[8];
    flash_union_buffer[18].uint32_type =  LAT_data1[9];
    flash_union_buffer[19].uint32_type =  LOG_data1[9];
    flash_union_buffer[20].uint32_type =  LAT_data1[10];
    flash_union_buffer[21].uint32_type =  LOG_data1[10];

    flash_union_buffer[22].uint32_type =  LAT_data1[11];
    flash_union_buffer[23].uint32_type =  LOG_data1[11];
    flash_union_buffer[24].uint32_type =  LAT_data1[12];
    flash_union_buffer[25].uint32_type =  LOG_data1[12];
    flash_union_buffer[26].uint32_type =  LAT_data1[13];
    flash_union_buffer[27].uint32_type =  LOG_data1[13];//科二13个点


    flash_union_buffer[28].uint32_type =  LAT_data1[14];
    flash_union_buffer[29].uint32_type =  LOG_data1[14];//科二15个点

    // 检查并擦除目标页
    if(flash_check(FLASH_SECTION_INDEX1, FLASH_GPS_INDEX1))
    {
        flash_erase_page(FLASH_SECTION_INDEX1, FLASH_GPS_INDEX1);
    }

    // 关键修正：立即将数据写入Flash
    flash_write_page_from_buffer(FLASH_SECTION_INDEX1, FLASH_GPS_INDEX1);
    ips200_show_string(172, 16*10, "CN flash");
}

/************************************************
 *  函数名称: 将Flash的数据读出：FLASH_READ_KM2(void)
 *  功能说明：将KM2的Flash的数据读出
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月16日
 *  备   注：
 ************************************************/
void FLASH_READ_KM2(void)
{
    //读取操作
    if(flash_check(FLASH_SECTION_INDEX1, FLASH_GPS_INDEX1))
    {
        flash_read_page_to_buffer(FLASH_SECTION_INDEX1, FLASH_GPS_INDEX1);

        LAT_data1[0] = flash_union_buffer[0].uint32_type;
        LOG_data1[0] = flash_union_buffer[1].uint32_type;
        LAT_data1[1] = flash_union_buffer[2].uint32_type;
        LOG_data1[1] = flash_union_buffer[3].uint32_type;
        LAT_data1[2] = flash_union_buffer[4].uint32_type;
        LOG_data1[2] = flash_union_buffer[5].uint32_type;
        LAT_data1[3] = flash_union_buffer[6].uint32_type;
        LOG_data1[3] = flash_union_buffer[7].uint32_type;
        LAT_data1[4] = flash_union_buffer[8].uint32_type;
        LOG_data1[4] = flash_union_buffer[9].uint32_type;

        LAT_data1[5] = flash_union_buffer[10].uint32_type;
        LOG_data1[5] = flash_union_buffer[11].uint32_type;
        LAT_data1[6] = flash_union_buffer[12].uint32_type;
        LOG_data1[6] = flash_union_buffer[13].uint32_type;
        LAT_data1[7] = flash_union_buffer[14].uint32_type;
        LOG_data1[7] = flash_union_buffer[15].uint32_type;
        LAT_data1[8] = flash_union_buffer[16].uint32_type;
        LOG_data1[8] = flash_union_buffer[17].uint32_type;
        LAT_data1[9] = flash_union_buffer[18].uint32_type;
        LOG_data1[9] = flash_union_buffer[19].uint32_type;
        LAT_data1[10] = flash_union_buffer[20].uint32_type;
        LOG_data1[10] = flash_union_buffer[21].uint32_type;

        LAT_data1[11] = flash_union_buffer[22].uint32_type;
        LOG_data1[11] = flash_union_buffer[23].uint32_type;
        LAT_data1[12] = flash_union_buffer[24].uint32_type;
        LOG_data1[12] = flash_union_buffer[25].uint32_type;
        LAT_data1[13] = flash_union_buffer[26].uint32_type;
        LOG_data1[13] = flash_union_buffer[27].uint32_type;

        LAT_data1[14] = flash_union_buffer[28].uint32_type;
        LOG_data1[14] = flash_union_buffer[29].uint32_type;

        ips200_show_string(172, 16*11, "OK flash");
    }
    else
    {
        ips200_show_string(172, 16*11, "NO flash");
    }
}

/*========================================科三Flash==========================================*/

/************************************************
 *  函数名称: 将数据存入Flash：FLASH_Get_KM3(void)
 *  功能说明：将数据写入KM3的Flash
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月16日
 *  备   注：
 ************************************************/
void FLASH_Get_KM3(void)
{

}

/************************************************
 *  函数名称: 将Flash的数据读出：FLASH_READ_KM3(void)
 *  功能说明：将KM3的Flash的数据读出
 *  参数说明：
 *  参   数：无
 *  函数返回：无
 *  修改时间：2025年6月16日
 *  备   注：
 ************************************************/
void FLASH_READ_KM3(void)
{

}

