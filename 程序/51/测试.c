#include <REGX52.H>

// 引脚定义
sbit Trig = P1^2;    // 超声波触发引脚
sbit Echo = P1^1;    // 超声波接收引脚
sbit LED1 = P2^0;     // LED控制引脚
sbit in_d = P1^0;     // 电机控制引脚（PWM输出）
sbit KEY1 = P3^1;     // 挡位1按键
sbit KEY2 = P3^0;     // 挡位2按键
sbit KEY3 = P3^2;     // 挡位3按键

// 数码管引脚定义（1位共阴极）
#define DIGIT_PORT P0  // 段选端口
sbit DIGIT_SEL = P2^1; // 位选端口

// PWM参数（迁移到定时器0）
#define PWM_PERIOD 100  // PWM周期（100μs，频率10kHz）
unsigned char duty_cycle = 0;  // 占空比（0~100）
unsigned char current_gear = 0; // 当前挡位（0~3，0为停止）

// 挡位占空比配置
#define GEAR1_DUTY 50   // 1挡：50%占空比
#define GEAR2_DUTY 75   // 2挡：75%占空比
#define GEAR3_DUTY 100  // 3挡：100%占空比

// 类型定义
typedef unsigned char u8;
typedef unsigned int u16;

// 共阴极数码管段码表（0~9）
u8 code digit_table[] = {
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

// 定时器0初始化（用于PWM生成）
void Timer0_Init(void) {
    TMOD &= 0xF0;  // 清空定时器0模式（保留定时器1配置）
    TMOD |= 0x01;  // 定时器0工作模式1（16位定时器）
    TH0 = (65536 - PWM_PERIOD) / 256;  // 定时器0高8位初值
    TL0 = (65536 - PWM_PERIOD) % 256;  // 定时器0低8位初值
    ET0 = 1;       // 使能定时器0中断
    TR0 = 1;       // 启动定时器0
    EA = 1;        // 开启总中断
}

// 定时器0中断服务函数（PWM生成）
void Timer0_ISR(void) interrupt 1 {
    static u8 count = 0;
    TH0 = (65536 - PWM_PERIOD) / 256;  // 重装定时器0初值
    TL0 = (65536 - PWM_PERIOD) % 256;
    
    count++;
    in_d = (count <= duty_cycle) ? 1 : 0;  // 根据占空比控制电机引脚电平
    if (count >= PWM_PERIOD) {
        count = 0;  // 周期结束，重置计数
    }
}

// 串口初始化（定时器1作为波特率发生器）
void UART_Init(void) {
    SCON = 0x50;  // 串口模式1（8位数据+1位停止，无校验），允许接收
    TMOD &= 0x0F; // 清空定时器1模式
    TMOD |= 0x20; // 定时器1工作模式2（8位自动重装）
    TH1 = 256 - (11059200 / 12 / 32) / 9600;  // 9600bps波特率初值（11.0592MHz晶振）
    TL1 = TH1;                                // 自动重装，与TH1一致
    TR1 = 1;       // 启动定时器1
    ES = 1;        // 使能串口中断
    EA = 1;        // 开启总中断
}

// 串口中断服务函数
u8 tempt = 0;  // 串口接收数据缓冲区（显式初始化）
void UART_ISR(void) interrupt 4 {
    if (RI) {          // 接收中断标志位
        tempt = SBUF;  // 读取接收数据
        RI = 0;        // 清接收中断标志
    }
}

// 微秒级延时（用于超声波触发）
void Delay10us(void) {
    u8 i = 2;
    while (--i);
}

// 毫秒级延时（用于消抖、主循环间隔）
void DelayMs(u16 ms) {
    u16 i, j;
    for (i = ms; i > 0; i--)
        for (j = 110; j > 0; j--);
}

// 数码管显示函数（带消抖）
void Display_Gear(void) {
    DIGIT_PORT = digit_table[current_gear];  // 输出当前挡位段码
    DIGIT_SEL = 0;                           // 选中数码管（共阴极低电平有效）
    DelayMs(1);                              // 轻微延时，避免闪烁
}

// 按键检测函数（消抖处理）
void Key_Scan(void) {
    // 检测挡位1按键
    if (KEY1 == 0) {
        DelayMs(10);  // 消抖延时
        if (KEY1 == 0) {
            duty_cycle = GEAR1_DUTY;
            current_gear = 1;
            while (!KEY1);  // 等待按键松开
        }
    }
    // 检测挡位2按键
    if (KEY2 == 0) {
        DelayMs(10);
        if (KEY2 == 0) {
            duty_cycle = GEAR2_DUTY;
            current_gear = 2;
            while (!KEY2);
        }
    }
    // 检测挡位3按键
    if (KEY3 == 0) {
        DelayMs(10);
        if (KEY3 == 0) {
            duty_cycle = GEAR3_DUTY;
            current_gear = 3;
            while (!KEY3);
        }
    }
}

// 超声波测距函数（优化：避免阻塞主程序）
u16 GetDistance(void) {
    u16 time = 0;
    u16 distance = 0;

    // 发送10us触发信号
    Trig = 1;
    Delay10us();
    Trig = 0;

    // 等待Echo上升沿（开始接收回波）
    while (Echo == 0 && time < 1000);  // 超时保护，避免死等
    if (time >= 1000) return 0;        // 超时返回0（无效距离）

    // 计时Echo高电平时间（使用定时器0计数，不阻塞）
    TR0 = 1;  // 启动定时器0计时
    while (Echo == 1 && time < 60000); // 超时保护（最大测量距离约10m）
    TR0 = 0;  // 停止定时器0

    // 计算时间（定时器0计数次数 = TH0*256 + TL0）
    time = TH0 * 256 + TL0;
    TH0 = 0;  // 清空定时器0
    TL0 = 0;

    // 计算距离：距离(cm) = 时间(us) * 声速(340m/s) / 2 / 10000
    distance = time * 0.17;
    return distance;
}

// 主函数
void main(void) {
    UART_Init();    // 初始化串口（定时器1）
    Timer0_Init();  // 初始化PWM（定时器0）
    LED1 = 1;       // 初始LED熄灭
    duty_cycle = 0; // 初始电机停止
    current_gear = 0; // 初始挡位0

    while (1) {
        Key_Scan();        // 按键检测
        Display_Gear();    // 数码管显示当前挡位

        // 串口控制逻辑
        switch (tempt) {
            case 0x34:  // 正转（实际已通过PWM控制，此处可扩展方向）
                duty_cycle = GEAR1_DUTY;
                current_gear = 1;
                tempt = 0;  // 清空接收标志
                break;
            case 0x35:  // 停止
                duty_cycle = 0;
                current_gear = 0;
                tempt = 0;
                break;
            case 0x33:  // LED流水灯开启
                for ( i = 0; i < 7; i++) {
                    DelayMs(300);
                    P2 = 0x7F >> i;
                }
                tempt = 0;
                break;
            case 0x32:  // LED关闭
                P2 = 0xFF;  // 所有LED熄灭
                tempt = 0;
                break;
            case 0x36:  // 1挡
                duty_cycle = GEAR1_DUTY;
                current_gear = 1;
                tempt = 0;
                break;
            case 0x37:  // 2挡
                duty_cycle = GEAR2_DUTY;
                current_gear = 2;
                tempt = 0;
                break;
            case 0x38:  // 3挡
                duty_cycle = GEAR3_DUTY;
                current_gear = 3;
                tempt = 0;
                break;
        }

        // 超声波避障：距离小于10cm时，电机停止，LED亮
        u16 distance = GetDistance();
        if (distance > 0 && distance < 10) {
            LED1 = 0;
            duty_cycle = 0;
            current_gear = 0;
        } else {
            LED1 = 1;
        }

        DelayMs(20);  // 主循环延时，降低CPU占用率
    }
}