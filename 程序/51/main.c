#include <REGX52.H>
#include<intrins.h>
 
sbit Trig = P1^2;   // 超声波触发引脚
sbit Echo = P1^1;   // 超声波接收引脚
sbit LED1 = P2^0;    // LED控制引脚
sbit in_d=P1^0;		//电机
sbit KEY1 = P3^1;    // 挡位1按键
sbit KEY2 = P3^0;    // 挡位2按键
sbit KEY3 = P3^2;    // 挡位3按键
sbit RS=P2^6;				// lcd rs口
sbit RW=P2^5;				// lcd rw口
sbit E=P2^7;				// lcd e口

// 数码管引脚定义（1位共阴极数码管）
#define DIGIT_PORT P0 // 段选端口
sbit DIGIT_SEL = P2^1; // 位选端口（若多位可扩展）

// 定义PWM参数（周期=100μs，频率=10kHz）
#define PWM_PERIOD 100  // PWM周期（定时器计数次数）
unsigned char duty_cycle = 0; // 占空比（0~PWM_PERIOD）
unsigned char current_gear = 0; // 当前挡位（0~3，0为停止）

// 挡位占空比配置（可根据实际需求调整）
#define GEAR1_DUTY 50   // 挡位2：50%占空比（中低速）
#define GEAR2_DUTY 75   // 挡位3：75%占空比（中高速）
#define GEAR3_DUTY 100  // 挡位4：100%占空比（高速）

typedef char u8;
typedef int u16;

// 共阴极数码管段码表（0~9）
unsigned char code digit_table[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};

void Timer1_Init(void) {
    TMOD &= 0xF0;  // 清空定时器1模式
    TMOD |= 0x01;  // 定时器1工作模式1（16位自动重装）
    TH0 = (65536 - PWM_PERIOD) / 256;  // 初值高8位
    TL0 = (65536 - PWM_PERIOD) % 256;  // 初值低8位
    ET0 = 1;       // 使能定时器1中断
    TR0 = 1;       // 启动定时器1
    EA = 1;        // 开启总中断
}

// 定时器1中断服务函数（PWM生成）电机的快慢
void Timer1_ISR(void) interrupt 1 {
    static unsigned char count = 0;
    TH0 = (65536 - PWM_PERIOD) / 256;  // 重装初值
    TL0 = (65536 - PWM_PERIOD) % 256;
    
    count++;
    if (count <= duty_cycle) {
        in_d = 1;  // 高电平
    } else {
        in_d = 0;  // 低电平
    }
    if (count >= PWM_PERIOD) {
        count = 0;    // 周期结束，重置计数
    }
}

//语音模块配置接口
void UART_Init()
{
	
	SCON = 0x50;//配置串口的通信方式
	TMOD &= 0x0F;
	TMOD |= 0x20;
	TH1 = 256 - (11059200 / 12 / 32) / 9600;//装载需要计算
	TL1 = TH1;
	TR1 = 1;
	ES = 1;
//	PS=1;
	EA = 1;
}

//微秒
void Delay10us() {
    unsigned char i;
    i = 2;
    while (--i);
}

void delay()
{
	_nop_();
	_nop_();
	_nop_();
}
// 毫秒级延时（用于主循环间隔）
void DelayMs(unsigned int ms) 
{
    unsigned int i, j;
    for (i = ms; i > 0; i--)
        for (j = 110; j > 0; j--);
}

// 数码管显示函数（显示当前挡位）
void Display_Gear(void) 
{
    DIGIT_PORT = digit_table[current_gear]; // 输出段码
    DIGIT_SEL = 0; // 选中数码管（共阴极位选低电平有效，若为共阳极需改为1）
		DelayMs(1);
}

// 测量距离并返回（单位：cm）
unsigned int GetDistance() 
{
    unsigned int time = 0;
    unsigned int distance = 0;

    // 发送触发信号：10us高电平
    Trig = 1;
    Delay10us();
    Trig = 0;

    // 等待Echo上升沿（开始接收回波）
    while (Echo == 0);
    // 计时Echo高电平持续时间（回波接收时间）
    while (Echo == 1) {
        time++;
        Delay10us();
        if (time > 60000) break; // 超时保护，避免死循环
    }

    // 计算距离：距离(cm) = 时间(us) * 声速(340m/s) / 2 / 10000
    distance = time * 0.17;
    return distance;
}

void lcd_w_cmd(unsigned char com)//写命令
{
	DelayMs(1);
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
	DelayMs(1);
}

void lcd_w_dat(unsigned char dat)//写数据
{
	DelayMs(1);
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
	DelayMs(1);  // 修正：delayms -> delaysms
}

void lcd_init()  // 初始化函数名
{
 lcd_w_cmd(0x38);  // 改为5×8点阵，更通用
 lcd_w_cmd(0x0c);  // 开显示+显示光标
 lcd_w_cmd(0x01);  // 清屏
 lcd_w_cmd(0x06);  // 光标右移，整屏不移动
 lcd_w_cmd(0x80);  // 光标定位到第一行第1列
}

void lcd_1(unsigned char *a)
{
	//unsigned char lcd2[]="???!";
	P0=0xff;
	DelayMs(1);
	lcd_w_cmd(0x83);// 光标定位到第一行第4列
	//lcd_w_cmd(0xc5);
	DelayMs(1); 
	while(*a != '\0') 
		{
		lcd_w_dat(*a);
		a++;
		DelayMs(1);
		}
	
}

// 按键检测函数（消抖处理）
void Key_Scan(void) {
    // 检测挡位1按键
    if (KEY1 == 0) {
        DelayMs(10);
        if (KEY1 == 0) {
            duty_cycle = GEAR1_DUTY;
            current_gear = 1;
						lcd_1("1");
            while (!KEY1);
        }
    }
    // 检测挡位2按键
    if (KEY2 == 0) {
        DelayMs(10);
        if (KEY2 == 0) {
            duty_cycle = GEAR2_DUTY;
            current_gear = 2;
						lcd_1("2");
            while (!KEY2);
        }
    }
    // 检测挡位3按键
    if (KEY3 == 0) {
        DelayMs(10);
        if (KEY3 == 0) {
            duty_cycle = GEAR3_DUTY;
            current_gear = 3;
						lcd_1("3");
            while (!KEY3);
        }
    }
}


unsigned int tempt,i;
void main(void)
{
//	u8 dir=0;
	lcd_init();
  UART_Init();                  //置T0定时工作方式1
	Timer1_Init();
  EA = 1;  //开总中断
	LED1 = 1;      // 初始状态：LED熄灭
  duty_cycle = 0; // 初始转速为0（电机停止）
	current_gear = 0; // 初始挡位为0
	while(1)
	{
		 Key_Scan();  // 循环检测按键
     Display_Gear(); // 循环显示当前挡位

		if(tempt==0x34)//正转
		{
			DelayMs(1000);
			duty_cycle = GEAR3_DUTY;
			current_gear = 3;
			lcd_1("3");
			tempt=0x00;
		}
		if(tempt==0x35)//停止
		{
			DelayMs(1000);
			duty_cycle=0;
			current_gear = 0;
			lcd_1("0");
			tempt=0x00;
		}
		if(tempt==0x33)//LED灯开启
		{
			DelayMs(1000);
			for(i=0;i<7;i++)
			{DelayMs(300);
			P2=0x7f>>i;}
			tempt=0x00;
		}
		if(tempt==0x32)//LED灯关闭
		{
			DelayMs(1000);
			for(i=1;i<9;i++)
			{DelayMs(300);
			P2=~(0x7f<<i);}
			tempt=0x00;
		}
		if(tempt==0x36)//1档
		{
			DelayMs(1000);
			duty_cycle = GEAR1_DUTY;
      current_gear = 1;
			lcd_1("1");
			tempt=0x00;
		}
		if(tempt==0x37)//2档
		{
			DelayMs(1000);
			duty_cycle = GEAR2_DUTY;
      current_gear = 2;
			lcd_1("2");
			tempt=0x00;
		}
		if(tempt==0x38)//3档
		{
			DelayMs(1000);
			duty_cycle = GEAR3_DUTY;
      current_gear = 3;
			lcd_1("3");
			tempt=0x00;
		}
    // 根据距离控制LED
    if (GetDistance()< 2) // 距离小于10cm，LED亮
		{      
        LED1 = 0;
				duty_cycle=0;
				current_gear = 0;
				lcd_1("0");
				
     } 
		 else 
		 {             // 距离大于10cm，LED灭
        LED1 = 1;

     }
				DelayMs(20); // 延时刷新，避免频繁检测

		}
}

void InterruptUART() interrupt 4
{
	if(RI)
	{
		tempt = SBUF;
		RI= 0;
//		led1 =0;
	  
	}
}

