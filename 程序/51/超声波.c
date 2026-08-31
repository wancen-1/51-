#include <reg52.h>

// 定义引脚
sbit Trig = P1^0;   // 超声波触发引脚
sbit Echo = P1^1;   // 超声波接收引脚
sbit LED = P2^0;    // LED控制引脚
sbit in_d=P1^0;		//电机

// 10微秒延时函数（11.0592MHz晶振）
void Delay10us() {
    unsigned char i;
    i = 2;
    while (--i);
}

// 毫秒级延时（用于主循环间隔）
void DelayMs(unsigned int ms) {
    unsigned int i, j;
    for (i = ms; i > 0; i--)
        for (j = 110; j > 0; j--);
}

// 测量距离并返回（单位：cm）
unsigned int GetDistance() {
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

void main() {
    unsigned int dis;
		in_d=1;
    while (1) {
        dis = GetDistance(); // 获取当前距离
        
        // 根据距离控制LED
        if (dis < 2) {      // 距离小于10cm，LED亮
            LED = 0;
						in_d=0;
        } else {             // 距离大于10cm，LED灭
            LED = 1;
						in_d=1;
        }
        
        DelayMs(200); // 延时刷新，避免频繁检测
    }
}