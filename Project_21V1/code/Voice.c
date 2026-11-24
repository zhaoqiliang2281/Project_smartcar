/*
 * Voice.c
 *
 *  Created on: 2026年4月1日
 *      Author: liuliangfang
 */

#include "zf_common_headfile.h"

int16 Mic_Voice[2][Mic_Voice_Arr_Length];
uint16 Voice_Arr_Position=0;//现在应该放在数组的哪个地方
Position_enum Position=Zero;//现在应该往哪个数组里放数据
uint8 Voice_Get_Flag=0;//现在能不能发


void Voice_Init()
{
    adc_init(Mic1,Resolution);
    adc_init(Mic2,Resolution);
//    pit_init(CCU61_CH1,125);//8khz
}
void Voice_Get()
{
    int16 sample;
    switch(Position)
    {
        case Zero:
            sample = (adc_convert(Mic1) + adc_convert(Mic2)) / 2 - 1960;
            sample <<= 4;   // 放大16倍，范围 -32000 ~ 32000
            Mic_Voice[Position][Voice_Arr_Position] = sample;
            break;
        case One:
            sample = (adc_convert(Mic1) + adc_convert(Mic2)) / 2 - 1960;
            sample <<= 4;   // 放大16倍，范围 -32000 ~ 32000
            Mic_Voice[Position][Voice_Arr_Position] = sample;
            break;
    }
    seekfree_assistant_oscilloscope_data.data[0]=adc_convert(Mic1)-1960;
    seekfree_assistant_oscilloscope_data.data[1]=adc_convert(Mic2)-1960;
    Voice_Arr_Position++;
/*    // 4. 打印两个麦克风的值（用于调试）
    printf("%d, %d\n",
           seekfree_assistant_oscilloscope_data.data[0],
           seekfree_assistant_oscilloscope_data.data[1]);*/

    if(Voice_Arr_Position==Mic_Voice_Arr_Length)//数组放满了
    {
        Voice_Arr_Position=0;
        switch(Position) {
            case Zero:Position=One;break;
            case One:Position=Zero;break;
        }
        Voice_Get_Flag=1;
    }
}






