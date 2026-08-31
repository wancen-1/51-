#include<reg52.h>
#include<intrins.h>
sbit RS=P2^6;
sbit RW=P2^5;
sbit E=P2^7;
sbit KEY3=P3^2;
sbit KEY4=P3^3;

void delay()
{
	_nop_();
	_nop_();
	_nop_();
}

void delaysms(unsigned int ms) {
    unsigned int i, j;
    for(; ms > 0; ms--) {
        for(i = 128; i > 0; i--) {
            for(j = 1; j > 0; j--);
        }
    }
}

//void time_1(char i)
//{
//	char j;
//	for(j=0;j<i;j++)
//	{
//		TH0=(8192-50000)/32;
//		TL0=(8192-50000)%32;
//		TR0=1;
//		while(!TF0);
//		TF0=0;
//	}
//}

void lcd_w_cmd(unsigned char com)//写命令
{
	delaysms(255);
	RS=0;
	delay();
	RW=0;
	delay();
	E=1;
	delay();
	P0=com;
	delay();
	E=0;
	delay();
	RW=1;
	delaysms(1);
}

void lcd_w_dat(unsigned char dat)//写数据
{
	delaysms(255);
	RS=1;
	delay();
	RW=0;
	delay();
	E=1;
	delay();
	P0=dat;
	delay();
	E=0;
	delay();
	RW=1;
	delaysms(1);  // 修正：delayms -> delaysms
}

void lcd_init()  // 初始化函数名
{
 lcd_w_cmd(0x38);  // 改为5×8点阵，更通用
 lcd_w_cmd(0x0e);  // 开显示+显示光标
 lcd_w_cmd(0x01);  // 清屏
 lcd_w_cmd(0x06);  // 光标右移，整屏不移动
 lcd_w_cmd(0x80);  // 光标定位到第一行第1列
}

void main()
{
		TMOD=0x00;
		EX0=1;
		PX0=1;
		IT0=1;
		EX1=1;
		PX1=1;
		IT1=1;
		EA=1;
		P2=0xfe;
	
		while(1)
		{
//			time_1(100);
			
		}

}
void int_0() interrupt 0
{
	if(KEY3==0)
	{
			delaysms(1);
			if(KEY3==0)
			{
				unsigned char lcd[]="HELLO";
				//unsigned char lcd2[]="???!";
				unsigned char i;
				P0=0xff;
				lcd_init();     // 修正：lcd_int -> lcd_init
				delaysms(255);  // 修正：delayms -> delaysms
				lcd_w_cmd(0x83);// 光标定位到第一行第4列
				//lcd_w_cmd(0xc5);
				delaysms(255);  // 修正：delayms -> delaysms
				for(i=0;lcd[i]!='\0';i++)
				{
					lcd_w_dat(lcd[i]);
					delaysms(200); // 修正：delayms -> delaysms
				}
			}
	
	}
}

void int_1() interrupt 2
{
		if(KEY4==0)
	{
			delaysms(1);
			if(KEY4==0)
			{
				unsigned char lcd[]="";
				//unsigned char lcd2[]="???!";
				unsigned char i;
				P0=0xff;
				lcd_init();     // 修正：lcd_int -> lcd_init
				delaysms(255);  // 修正：delayms -> delaysms
				lcd_w_cmd(0x83);// 光标定位到第一行第4列
				//lcd_w_cmd(0xc5);
				delaysms(255);  // 修正：delayms -> delaysms
				for(i=0;lcd[i]!='\0';i++)
				{
					lcd_w_dat(lcd[i]);
					delaysms(200); // 修正：delayms -> delaysms
				}
			}
	
	}
}