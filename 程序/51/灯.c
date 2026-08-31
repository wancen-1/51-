#include <REGX52.H>
#include <intrins.h>
sbit key1=P3^2;
sbit key2=P3^3;
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

void main()
{
	
	while(1)
	{
		time_1(100);
		P2=_crol_(P2,1);
	}
}
void int_0() interrupt 0
{
	char i,j;
	j=P2;
	P2=0x7f;
	for(i=1;i<9;i++)
	{
		time_1(115);
		P2 = ~(0xff << (8 - i));
	}
	while(!key1);
	P2=j;
}

void int_1() interrupt 2
{
		char i,j;
	j=P2;
	P2=0x7f;
	for(i=0;i<3;i++)
	{
		P2=0x00;
		time_1(100);
		P2=0xff;
		time_1(100);
	}
	while(!key2);
	P2=j;
}