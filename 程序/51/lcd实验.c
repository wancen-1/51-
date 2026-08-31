#include <regx52.h>

#define key P1
#define smg P0

int gsmg[17]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c,0x39,0x5e,0x79,0x71};

void delay( int ten)
{
	while(ten--);
}

int key11(void)
{
	int value=0;
	
	key=0xf7;//第一列
	if(key!=0xf7)
	{
		delay(1000);
		switch(key)
		{
			case 0x77:value=1;break;
			case 0xb7:value=5;break;
			case 0xd7:value=9;break;
			case 0xe7:value=13;break;
		}
	}
	while(key!=0xf7);
	
	key=0xfb;//第二列
	if(key!=0xfb)
	{
		delay(1000);
		switch(key)
		{
			case 0x7b:value=2;break;
			case 0xbb:value=6;break;
			case 0xdb:value=10;break;
			case 0xeb:value=14;break;
		}
	}
	while(key!=0xfb);
	
	key=0xfd;//第三列
	if(key!=0xfd)
	{
		delay(1000);
		switch(key)
		{
			case 0x7d:value=3;break;
			case 0xbd:value=7;break;
			case 0xdd:value=11;break;
			case 0xed:value=15;break;
		}
	}
	while(key!=0xfd);
	
	key=0xfe;//第四列
	if(key!=0xfe)
	{
		delay(1000);
		switch(key)
		{
			case 0x7e:value=4;break;
			case 0xbe:value=8;break;
			case 0xde:value=12;break;
			case 0xee:value=16;break;
		}
	}
	while(key!=0xfe);
	return value;
}

void main()
{

	int key=0;
	while(1)
	{
		key=key11();
		if(key!=0)
		{
			smg=gsmg[key-1];
		}
		
	}
}