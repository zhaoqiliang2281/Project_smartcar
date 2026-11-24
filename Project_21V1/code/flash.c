/*
 * flash.c
 *
 *  Created on: 2025年6月13日
 *      Author: LiangZhaoqi
 */

#include "zf_common_headfile.h"


uint32 LAT_data1[Point_MAX] = {0};  //纬度存储数组
uint32 LOG_data1[Point_MAX] = {0};  //经度存储数组

/*========================================第一Flash==========================================*/
/************************************************
 *  函数名称: 写入数据到Flash
 *  功能说明: 将KM1的Flash数据写入
 *  参数说明:
 *  返回值:
 *  修改日期：2025年6月16日
 *  备注
 ************************************************/
void FLASH_Get_KM1(void)
{
    // 清空缓存数组
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


    // 检查扇区是否有效
    if(flash_check(FLASH_SECTION_INDEX, FLASH_GPS_INDEX))
    {
        flash_erase_page(FLASH_SECTION_INDEX, FLASH_GPS_INDEX);
    }

    //将数据写入Flash
    flash_write_page_from_buffer(FLASH_SECTION_INDEX, FLASH_GPS_INDEX);

    ips200_show_string(172, 16*10, "CN flash");
}

/************************************************
 *  函数名称: 从Flash读取数据
 *  功能说明: 从KM1的Flash读取数据
 *  参数说明:
 *  返回值:
 *  修改日期：2025年6月16日
 *  备注
 ************************************************/
void FLASH_READ_KM1(void)
{
    //读取数据
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

        ips200_show_string(172, 16*11, "OK flash");
    }
    else
    {
        ips200_show_string(172, 16*11, "NO flash");
    }
}

/*========================================第二Flash==========================================*/

/************************************************
 *  函数名称: 写入数据到Flash
 *  功能说明: 将KM2的Flash数据写入
 *  参数说明:
 *  返回值:
 *  修改日期：2025年6月16日
 *  备注
 ************************************************/
void FLASH_Get_KM2(void)
{
    // 清空缓存数组
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
    flash_union_buffer[27].uint32_type =  LOG_data1[13];//第二13个点


    flash_union_buffer[28].uint32_type =  LAT_data1[14];
    flash_union_buffer[29].uint32_type =  LOG_data1[14];//第二15个点
    flash_union_buffer[30].uint32_type =  LAT_data1[15];
    flash_union_buffer[31].uint32_type =  LOG_data1[15];//第二15个点

    /* KM2 INS坐标: float(m) -> int32(cm), 存到索引32~63 */
    for(int i = 0; i < 16; i++) {
        flash_union_buffer[32 + i].uint32_type = (uint32)(int32)(KM2_INS_X[i] * 100.0f);
        flash_union_buffer[48 + i].uint32_type = (uint32)(int32)(KM2_INS_Y[i] * 100.0f);
    }

    // 检查扇区是否有效
    if(flash_check(FLASH_SECTION_INDEX1, FLASH_GPS_INDEX1))
    {
        flash_erase_page(FLASH_SECTION_INDEX1, FLASH_GPS_INDEX1);
    }

    // 确认数据正确后再写入Flash
    flash_write_page_from_buffer(FLASH_SECTION_INDEX1, FLASH_GPS_INDEX1);
    ips200_show_string(172, 16*10, "CN flash");
}

/************************************************
 *  函数名称: 从Flash读取数据
 *  功能说明: 从KM2的Flash读取数据
 *  参数说明:
 *  返回值:
 *  修改日期：2025年6月16日
 *  备注
 ************************************************/
void FLASH_READ_KM2(void)
{
    //读取数据
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
        LAT_data1[15] = flash_union_buffer[30].uint32_type;
        LOG_data1[15] = flash_union_buffer[31].uint32_type;

        /* KM2 INS coordinates */
        for(int i = 0; i < 16; i++) {
            KM2_INS_X[i] = (float)((int32)flash_union_buffer[32 + i].uint32_type) / 100.0f;
            KM2_INS_Y[i] = (float)((int32)flash_union_buffer[48 + i].uint32_type) / 100.0f;
        }

        ips200_show_string(172, 16*11, "OK flash");
    }
    else
    {
        ips200_show_string(172, 16*11, "NO flash");
    }
}

/*========================================第三Flash==========================================*/

/************************************************
 *  函数名称: 写入数据到Flash
 *  功能说明: 将KM3的Flash数据写入
 *  参数说明:
 *  返回值:
 *  修改日期：2025年6月16日
 *  备注
 ************************************************/
void FLASH_Get_KM3(void)
{

}

/************************************************
 *  函数名称: 从Flash读取数据
 *  功能说明: 从KM3的Flash读取数据
 *  参数说明:
 *  返回值:
 *  修改日期：2025年6月16日
 *  备注
 ************************************************/
void FLASH_READ_KM3(void)
{

}
