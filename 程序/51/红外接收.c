#include <reg52.h>

typedef unsigned char uchar;
typedef unsigned int uint;

sbit IR_IN = P3^2;    // 红外接收引脚（INT0）
sbit Relay = P1^0;    // 继电器控制引脚

uchar IR_Buf[4];      // 存储红外数据（地址、地址反码、数据、数据反码）
bit IR_Flag = 0;      // 红外解码完成标志

// 定时器0初始化（用于红外信号计时）
void Timer0_Init() {
    TMOD |= 0x01;     // 模式1（16位定时器）
    TH0 = 0;
    TL0 = 0;
    ET0 = 1;          // 开定时器0中断
    EA = 1;           // 开总中断
}

// 外部中断0初始化（捕捉红外起始信号）
void INT0_Init() {
    IT0 = 1;          // 下降沿触发
    EX0 = 1;          // 开外部中断0
    EA = 1;
}

// 红外解码函数（NEC协议）
void IR_Decode() {
    uchar i, j;
    uchar time = 0;
    
    IR_Flag = 0;
    TH0 = 0;
    TL0 = 0;
    TR0 = 1;          // 启动定时器
    
    // 等待起始码（9ms低电平+4.5ms高电平）
    while(!IR_IN && TH0 < 0x40); // 9ms低电平
    if(TH0 > 0x40) { TR0 = 0; return; }
    
    TH0 = 0;
    while(IR_IN && TH0 < 0x20);  // 4.5ms高电平
    if(TH0 > 0x20) { TR0 = 0; return; }
    
    // 解码32位数据
    for(i=0; i<4; i++) {
        for(j=0; j<8; j++) {
            TH0 = 0;
            while(!IR_IN && TH0 < 0x10); // 低电平（560us）
            if(TH0 > 0x10) { TR0 = 0; return; }
            
            TH0 = 0;
            while(IR_IN && TH0 < 0x20); // 高电平判断数据位
            time = TH0;
            IR_Buf[i] >>= 1;
            if(time > 0x10) IR_Buf[i] |= 0x80; // 1.68ms高电平=1，560us=0
        }
    }
    TR0 = 0;
    IR_Flag = 1;      // 解码完成
}

// 外部中断0服务函数（红外起始信号触发）
void INT0_ISR() interrupt 0 {
    IR_Decode();      // 开始解码
}

// 风扇控制函数
void Fan_Control(uint data) {
    static bit fan_state = 0; // 风扇状态（0=停，1=转）
    if(data == 0x00) {       // 假设红外按键数据为0x00（自定义按键）
        fan_state = !fan_state;
        Relay = fan_state ? 1 : 0; // 低电平触发继电器吸合
    }
}

void main() {
    Timer0_Init();
    INT0_Init();
    Relay = 0;         // 初始风扇关闭
    while(1) {
        if(IR_Flag) {
            if(IR_Buf[2] == ~IR_Buf[3]) { // 数据校验
                Fan_Control(IR_Buf[2]);
            }
            IR_Flag = 0;
        }
    }
}