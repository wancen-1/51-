#include <reg52.h>

// 定义引脚
sbit in_d = P1^0; // PWM输出引脚
sbit KEY1 = P3^1;    // 挡位1按键
sbit KEY2 = P3^0;    // 挡位2按键
sbit KEY3 = P3^2;    // 挡位3按键

// 数码管引脚定义（1位共阴极数码管）
#define DIGIT_PORT P0 // 段选端口
sbit DIGIT_SEL = P2^0; // 位选端口（若多位可扩展）

// 定义PWM参数（周期=100μs，频率=10kHz）
#define PWM_PERIOD 100  // PWM周期（定时器计数次数）
unsigned char duty_cycle = 0; // 占空比（0~PWM_PERIOD）
unsigned char current_gear = 0; // 当前挡位（0~3，0为停止）

// 挡位占空比配置（可根据实际需求调整）
#define GEAR1_DUTY 50   // 挡位2：50%占空比（中低速）
#define GEAR2_DUTY 75   // 挡位3：75%占空比（中高速）
#define GEAR3_DUTY 100  // 挡位4：100%占空比（高速）

// 共阴极数码管段码表（0~9）
unsigned char code digit_table[] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F  // 9
};

// 延时函数（简单延时，可根据实际调整）
void delay(unsigned int time) {
    unsigned int i, j;
    for (i = time; i > 0; i--)
        for (j = 110; j > 0; j--);
}

// 定时器1初始化（产生PWM信号）
void Timer1_Init(void) {
    TMOD &= 0x0F;  // 清空定时器1模式
    TMOD |= 0x10;  // 定时器1工作模式1（16位自动重装）
    TH1 = (65536 - PWM_PERIOD) / 256;  // 初值高8位
    TL1 = (65536 - PWM_PERIOD) % 256;  // 初值低8位
    ET1 = 1;       // 使能定时器1中断
    TR1 = 1;       // 启动定时器1
    EA = 1;        // 开启总中断
}

// 定时器1中断服务函数（PWM生成）
void Timer1_ISR(void) interrupt 3 {
    static unsigned char count = 0;
    TH1 = (65536 - PWM_PERIOD) / 256;  // 重装初值
    TL1 = (65536 - PWM_PERIOD) % 256;
    
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

// 数码管显示函数（显示当前挡位）
void Display_Gear(void) {
    DIGIT_PORT = digit_table[current_gear]; // 输出段码
    DIGIT_SEL = 0; // 选中数码管（共阴极位选低电平有效，若为共阳极需改为1）
}

// 按键检测函数（消抖处理）
void Key_Scan(void) {
    // 检测挡位2按键
    if (KEY1 == 0) {
        delay(20);
        if (KEY1 == 0) {
            duty_cycle = GEAR1_DUTY;
            current_gear = 1;
            while (!KEY1);
        }
    }
    // 检测挡位3按键
    if (KEY2 == 0) {
        delay(20);
        if (KEY2 == 0) {
            duty_cycle = GEAR2_DUTY;
            current_gear = 2;
            while (!KEY2);
        }
    }
    // 检测挡位4按键
    if (KEY3 == 0) {
        delay(20);
        if (KEY3 == 0) {
            duty_cycle = GEAR3_DUTY;
            current_gear = 3;
            while (!KEY3);
        }
    }
}



void main(void) {
    Timer1_Init();  // 初始化定时器1（PWM）
    duty_cycle = 0; // 初始转速为0（电机停止）
    current_gear = 0; // 初始挡位为0
    
    while (1) {
        Key_Scan();  // 循环检测按键
        Display_Gear(); // 循环显示当前挡位
    }
}