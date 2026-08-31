#include <REGX52.H>
#include<intrins.h>

// 硬件引脚定义
sbit Trig = P1^2;   // 超声波触发引脚
sbit Echo = P1^1;   // 超声波接收引脚
sbit LED1 = P2^0;   // LED控制引脚
sbit in_d=P1^0;     // 电机PWM输出
sbit KEY1 = P3^1;   // 挡位1按键
sbit KEY2 = P3^0;   // 挡位2按键
sbit KEY3 = P3^2;   // 挡位3按键
sbit RS=P2^6;       // LCD RS
sbit RW=P2^5;       // LCD RW
sbit E=P2^7;        // LCD E

// 数码管引脚定义
#define DIGIT_PORT P0 // 段选端口
sbit DIGIT_SEL = P2^1; // 位选端口

// PWM参数（周期=100μs，频率=10kHz）
#define PWM_PERIOD 100  // PWM周期
unsigned char duty_cycle = 0; // 占空比（0~PWM_PERIOD）
unsigned char current_gear = 0; // 当前挡位（0~3）

// 挡位占空比配置
#define GEAR1_DUTY 50   // 1档：50%
#define GEAR2_DUTY 75   // 2档：75%
#define GEAR3_DUTY 100  // 3档：100%

// 类型定义
typedef unsigned char u8;
typedef unsigned int u16;

// 共阴极数码管段码表（0~9）
unsigned char code digit_table[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};

// 全局变量（用于节流和状态标记）
u16 distance_check_cnt = 0;  // 超声波检测计数（节流用）
u8 uart_cmd_flag = 0;        // 串口指令标记（避免长延时）
u8 uart_cmd = 0;             // 存储串口指令
u8 led_flow_flag = 0;        // LED流水灯标记
u16 led_flow_cnt = 0;        // LED流水灯计数

// ==================== 定时器初始化（修正错误）====================
// 定时器0初始化（生成PWM，原代码错误用了Timer1的注释）
void Timer0_Init(void) {
    TMOD &= 0xF0;  // 清空定时器0模式
    TMOD |= 0x01;  // 定时器0工作模式1（16位）
    TH0 = (65536 - PWM_PERIOD) / 256;  // 初值高8位
    TL0 = (65536 - PWM_PERIOD) % 256;  // 初值低8位
    ET0 = 1;       // 使能定时器0中断
    TR0 = 1;       // 启动定时器0
    EA = 1;        // 开启总中断
}

// 定时器0中断服务函数（PWM生成）
void Timer0_ISR(void) interrupt 1 {
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

// ==================== 串口初始化 ====================
void UART_Init() {
    SCON = 0x50;       // 8位数据，可变波特率
    TMOD &= 0x0F;
    TMOD |= 0x20;      // 定时器1模式2（8位自动重装）
    TH1 = 0xFD;        // 9600波特率（11.0592MHz晶振）
    TL1 = 0xFD;
    TR1 = 1;           // 启动定时器1
    ES = 1;            // 使能串口中断
    EA = 1;            // 开启总中断
}

// ==================== 延时函数（精简）====================
// 微秒延时（超声波专用，精准）
void Delay10us() {
    unsigned char i;
    i = 2;
    while (--i);
}

// 短延时（LCD时序用）
void delay_nop() {
    _nop_();
    _nop_();
    _nop_();
}

// 毫秒级延时（仅用于必须的短延时，避免长延时）
void DelayMs(u16 ms) {
    u16 i, j;
    for (i = ms; i > 0; i--)
        for (j = 110; j > 0; j--);
}

// ==================== 硬件驱动函数 ====================
// 数码管显示函数（快速刷新，无长延时）
void Display_Gear(void) {
    DIGIT_PORT = digit_table[current_gear]; 
    DIGIT_SEL = 0;       // 选中数码管
    DelayMs(1);          // 极短延时，保证显示稳定
    DIGIT_SEL = 1;       // 取消选中，降低功耗
}

// 超声波测距（带超时保护，精简逻辑）
u16 GetDistance() {
		unsigned int timeout;
    u16 time = 0;
    u16 distance = 0;

    Trig = 1;
    Delay10us();
    Trig = 0;

    // 等待Echo上升沿（超时保护）
    
		timeout = 0;
    while (Echo == 0) {
        timeout++;
        if (timeout > 1000) return 999; // 超时返回无效值
    }

    // 计时Echo高电平（超时保护）
    timeout = 0;
    while (Echo == 1) {
        time++;
        Delay10us();
        timeout++;
        if (timeout > 60000) break;
    }

    distance = time * 0.17;
    return distance;
}

// LCD写命令
void lcd_w_cmd(u8 com) {
    DelayMs(1);
    RS=0;
    delay_nop();
    RW=0;
    delay_nop();
    E=1;
    delay_nop();
    P0=com;
    delay_nop();
    E=0;
    delay_nop();
    RW=1;
    DelayMs(1);
}

// LCD写数据
void lcd_w_dat(u8 dat) {
    DelayMs(1);
    RS=1;
    delay_nop();
    RW=0;
    delay_nop();
    E=1;
    delay_nop();
    P0=dat;
    delay_nop();
    E=0;
    delay_nop();
    RW=1;
    DelayMs(1);
}

// LCD初始化
void lcd_init() {
    lcd_w_cmd(0x38);
    lcd_w_cmd(0x0c);
    lcd_w_cmd(0x01);
    lcd_w_cmd(0x06);
    lcd_w_cmd(0x80);
}

// LCD显示单个字符（精简逻辑）
void lcd_show_char(u8 ch) {
    P0=0xff;
    DelayMs(1);
    lcd_w_cmd(0x83);
    DelayMs(1);
    lcd_w_dat(ch);
}

// ==================== 按键扫描（独立、快速）====================
void Key_Scan(void) {
    // 检测KEY1（消抖仅10ms，无阻塞）
    if (KEY1 == 0) {
        DelayMs(10);
        if (KEY1 == 0) {
            duty_cycle = GEAR1_DUTY;
            current_gear = 1;
            lcd_show_char('1');
            while (!KEY1); // 等待按键释放（仅等待释放，无额外延时）
        }
    }
    // 检测KEY2
    if (KEY2 == 0) {
        DelayMs(10);
        if (KEY2 == 0) {
            duty_cycle = GEAR2_DUTY;
            current_gear = 2;
            lcd_show_char('2');
            while (!KEY2);
        }
    }
    // 检测KEY3
    if (KEY3 == 0) {
        DelayMs(10);
        if (KEY3 == 0) {
            duty_cycle = GEAR3_DUTY;
            current_gear = 3;
            lcd_show_char('3');
            while (!KEY3);
        }
    }
}

// ==================== 串口中断（仅存指令，不处理）====================
void InterruptUART() interrupt 4 {
    if(RI) {
        uart_cmd = SBUF;  // 存储指令
        uart_cmd_flag = 1;// 置标记
        RI= 0;            // 清中断
    }
}

// ==================== 串口指令处理（无长延时）====================
void UART_Cmd_Process() {
    if (!uart_cmd_flag) return; // 无指令则返回
    
    switch(uart_cmd) {
        case 0x34: // 正转（3档）
            duty_cycle = GEAR3_DUTY;
            current_gear = 3;
            lcd_show_char('3');
            break;
        case 0x35: // 停止
            duty_cycle = 0;
            current_gear = 0;
            lcd_show_char('0');
            break;
        case 0x33: // LED流水灯开启
            led_flow_flag = 1;
            led_flow_cnt = 0;
            break;
        case 0x32: // LED流水灯关闭
            led_flow_flag = 0;
            P2 = 0xff; // 关闭所有LED
            break;
        case 0x36: // 1档
            duty_cycle = GEAR1_DUTY;
            current_gear = 1;
            lcd_show_char('1');
            break;
        case 0x37: // 2档
            duty_cycle = GEAR2_DUTY;
            current_gear = 2;
            lcd_show_char('2');
            break;
        case 0x38: // 3档
            duty_cycle = GEAR3_DUTY;
            current_gear = 3;
            lcd_show_char('3');
            break;
    }
    uart_cmd_flag = 0; // 清标记
    uart_cmd = 0;      // 清指令
}

// ==================== LED流水灯（非阻塞）====================
void LED_Flow_Process() {
		static unsigned char flow_i;
    if (!led_flow_flag) return;
    
    led_flow_cnt++;
    if (led_flow_cnt >= 300) { // 每300ms刷新一次（非阻塞）
        led_flow_cnt = 0;
        flow_i = 0;
        P2 = 0x7f >> flow_i;
        flow_i++;
        if (flow_i >= 7) flow_i = 0;
    }
}

// ==================== 主函数（精简、无长阻塞）====================
void main(void) {
		unsigned int dis; 
    lcd_init();
    UART_Init();
    Timer0_Init();  // 修正为定时器0
    
    LED1 = 1;       // 初始LED灭
    duty_cycle = 0; // 初始电机停止
    current_gear = 0;
    
    while(1) {
        Key_Scan();                // 优先扫描按键（核心）
        Display_Gear();            // 快速显示挡位
        UART_Cmd_Process();        // 处理串口指令（无长延时）
        LED_Flow_Process();        // 非阻塞处理LED流水灯
        
        // 超声波检测节流（每20次主循环检测一次，降低CPU占用）
        distance_check_cnt++;
        if (distance_check_cnt >= 20) {
            distance_check_cnt = 0;
            
						dis= GetDistance();
            if (dis < 2) { // 距离<2cm，电机停止，LED亮
                LED1 = 0;
                duty_cycle = 0;
                current_gear = 0;
                lcd_show_char('0');
            } else {        // 距离≥2cm，LED灭
                LED1 = 1;
            }
        }
        
        DelayMs(1); // 极短延时，降低CPU占用，不影响按键响应
    }
}