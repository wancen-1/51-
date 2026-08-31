#include <REGX52.H>
sbit LED1=P2^0;
sbit key=P3^1;
void delay(int i)
{
	while(i--);
}
void int_1(char i)
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
void main()
{
	TMOD=0x00;
	while(1)
	{
		if(key==0)//二十分之一
		{
			delay(10);
			if(key==0)
			{
				LED1=0;
				int_1(20);//1s
				LED1=1;
				int_1(1);
			}
		}
		LED1=0;
		int_1(1);
		LED1=1;
		int_1(20);
	}
}