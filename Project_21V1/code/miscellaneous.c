#include "zf_common_headfile.h"

uint8 Send_Flag=0;

void Send_Flag_Get(void)
{
    if(key4_flag==1&&(key4_state!=key4_state_last))//电平被改变，此时需要发数据了
    {
        key4_flag=0;
        Voice_Get_Flag=0;//这一段我不想发
        Send_Flag=1;
        ips200_show_rgb565_image(0, 16*2, (const uint16 *)DORO_START, 240, 320, 240,288, 0);//播放doro突脸，表示可以讲话了
        while(Voice_Get_Flag==0){}//阻塞在这里直到重新采满一个数组
        Fram_buffer_Creat(0);
        wifi_uart_send_buffer((uint8 *)Fram_Arr,Frame);
        Voice_Get_Flag=0;
        Buzzer_check(50);
    }
    else if(key4_flag==1&&(key4_state==key4_state_last))//电平被改变，此时需要发末尾数据包
    {
        Send_Flag=0;
        key4_flag=0;
        while(Voice_Get_Flag==0){}
        ips200_show_rgb565_image(0, 16*2, (const uint16 *)DORO_SLEEP, 240, 320, 240, 288, 0);//播放doro睡觉，表示讲话结束
        Fram_buffer_Creat(2);
        wifi_uart_send_buffer((uint8 *)Fram_Arr,Frame);
        Buzzer_check(50);
    }

}

void Send_Voice()
{
    if(Send_Flag==1)
    {
        if(Voice_Get_Flag==1)
        {
            Wifi_Uart_Continue_data_Send();
        }
    }
}



