#include<REGX52.H>
#include "oled.h"

void main()
{
    OLED_Init(); //初始化OLED
    OLED_Clear();
    Delay_1ms(2000);
    while (1)
    {
        OLED_ShowCHinese(0, 0, 0);  //爱
        OLED_ShowCHinese(16, 0, 1); //易
        OLED_ShowCHinese(32, 0, 2); //族
        OLED_ShowString(0, 3, "www.yizu.org", 16);
        Delay_1ms(2000);
    }
}