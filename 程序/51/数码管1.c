#include <regx52.h>

#define PORT P0 
unsigned char code a[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};
void disp(int i)
{
	while(i--);
}
void time_1(char i)
{
	char j;
	for(j=0;j<i;j++)
	{
		TH0=(8192-50000)/32;
		TL0=(8192-50000)%32;
		TR0=1;
		while(!TF0);
		TF0=0;
	}
}
void Nixie(unsigned char Location, Number) {
		P0 = 0x00;
    switch(Location){
        case 8:P2_4=0;P2_3=0;P2_2=0;break;
        case 7:P2_4=0;P2_3=0;P2_2=1;break;
        case 6:P2_4=0;P2_3=1;P2_2=0;break;
        case 5:P2_4=0;P2_3=1;P2_2=1;break;
        case 4:P2_4=1;P2_3=0;P2_2=0;break;
        case 3:P2_4=1;P2_3=0;P2_2=1;break;
        case 2:P2_4=1;P2_3=1;P2_2=0;break;
        case 1:P2_4=1;P2_3=1;P2_2=1;break;
    }
    P0 = a[Number];
    disp(20);
    P0 = 0x00;   // 消影：清空段码
}

void main() {

		TMOD=0x00;
		EX1=1;
		PX1=1;
		IT1=1;
		EA=1;
		
		
    while(1) 
		{
			time_1(20);
		}
	
}

void int_1() interrupt 2
{
	unsigned int i, j;
				for(i = 0; i < 10; i++) 
					{ 
            for(j = 0; j < 10; j++) 
						{
							unsigned int k;
								for(k=0;k<500;k++)
								{
								Nixie(1, i); // 第1位显示高位
                Nixie(2, j); 
								}
						}
					}
}