#include <reg51.h>

// 共阴极数码管段码表（0~9），dp位为0（不显示小数点）
unsigned char seg_code[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 
                           0x6D, 0x7D, 0x07, 0x7F, 0x6F};

// 计数变量：ge=个位（0~9），shi=十位（0~9），cnt=1ms中断计数器
unsigned char ge = 0, shi = 0;
unsigned int cnt = 0;

// 定时器0初始化：模式1（16位定时），1ms中断（11.0592MHz晶振）
void timer0_init(void) {
    TMOD |= 0x01;  // 定时器0工作模式1（GATE=0，C/T=0，M1=0，M0=1）
    TH0 = 0xFC;    // 初值高8位：65536 - 1000 = 64536 → 0xFC
    TL0 = 0x66;    // 初值低8位：0x66（11.0592MHz晶振，12T模式下1ms定时）
    ET0 = 1;       // 使能定时器0中断
    TR0 = 1;       // 启动定时器0
    EA = 1;        // 使能总中断
}

// 定时器0中断服务函数：1ms触发一次，累计1000次=1秒
void timer0_isr(void) interrupt 1 {
    TH0 = 0xFC;    // 重装初值（避免溢出后计时偏差）
    TL0 = 0x66;
    cnt++;
    
    if (cnt >= 1000) {  // 累计1000ms=1秒
        cnt = 0;        // 计数器清零
        ge++;           // 个位+1
        if (ge >= 10) { // 个位满10，进位到十位
            ge = 0;
            shi++;
            if (shi >= 10) { // 十位满10，重置为00
                shi = 0;
            }
        }
    }
}

// 静态显示函数：分别显示十位和个位
void display(void) {
	unsigned int i;
    // 显示十位：选中十位数码管（P2.1=0），输出十位段码
    P2 &= 0xFD;  // P2.1=0，其他位不变（0xFD=11111101）
    P0 = seg_code[shi];
    // 延时1ms（视觉暂留，避免闪烁）
    
    for (i = 0; i < 1000; i++);
    
    // 显示个位：选中个位数码管（P2.0=0），输出个位段码
    P2 |= 0x02;  // 关闭十位（P2.1=1）
    P2 &= 0xFE;  // P2.0=0（0xFE=11111110）
    P0 = seg_code[ge];
    for (i = 0; i < 1000; i++);
    
    P2 |= 0x01;  // 关闭个位（P2.0=1）
}

void main(void) {
    timer0_init();  // 初始化定时器0
    while (1) {
        display();  // 循环刷新显示
    }
}