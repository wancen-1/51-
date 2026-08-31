#include <reg52.h>

typedef unsigned char uchar;
typedef unsigned int uint;

sbit Relay = P1^0;  // 继电器控制引脚

// 串口初始化（9600波特率，晶振11.0592MHz）
void UART_Init() {
    TMOD |= 0x20;    // 定时器1工作模式2（8位自动重装）
    TH1 = 0xFD;      // 9600波特率初值
    TL1 = 0xFD;
    TR1 = 1;         // 启动定时器1
    SCON = 0x50;     // 串口工作模式1，允许接收
    EA = 1;          // 开总中断
    ES = 1;          // 开串口中断
}

// 风扇控制函数
void Fan_Control(uchar cmd) {
    switch(cmd) {
        case 01:    // 简化指令：'1'启动
            Relay = 1;
            break;
        case 00:    // 简化指令：'0'停止
            Relay = 0;
            break;
        default:
            break;
    }
}

// 串口中断服务函数（支持单字符或字符串指令）
void UART_ISR() interrupt 4 {
    int rec_data;
    
    if(RI) {         // 接收中断标志
        RI = 0;      // 清标志
        rec_data = SBUF;
        
        // 单字符指令处理
            Fan_Control(rec_data);

        }
}

void main() {
    UART_Init();     // 初始化串口
    Relay = 0;       // 初始状态：风扇关闭
    while(1);        // 主循环
}