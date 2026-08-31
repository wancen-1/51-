#include <reg52.h>

// 定义端口（根据硬件连接调整）
#define SEG_PORT P0    // 段选端口（a~dp）
#define BIT1 P2^0      // 第一个数码管位选（个位）
#define BIT2 P2^1      // 第二个数码管位选（十位）

// 共阴极数码管段码表（0~9），dp为小数点（此处未用，置0）
unsigned char seg_code[] = {0x3F,0x06,0x5B,0x4F,0x66,
                           0x6D,0x7D,0x07,0x7F,0x6F};

// 全局变量：当前显示的数字（0~99）
unsigned int num = 0;
// 中断计数：1ms中断一次，累计1000次为1秒
unsigned int count = 0;

/**
 * @brief 定时器0初始化：1ms中断（11.0592MHz晶振）
 */
void timer0_init() {
    // 16位定时器初值计算：1ms = 1000μs
    // 机器周期 = 12 / 晶振频率（11.0592MHz）≈ 1.085μs
    // 需计数次数 = 1000 / 1.085 ≈ 921，初值 = 65536 - 921 = 64615 = 0xFC67
    TMOD &= 0xF0;  // 清空定时器0模式
    TMOD |= 0x01;  // 定时器0工作模式1（16位自动重装）
    TH0 = 0xFC;    // 高8位初值
    TL0 = 0x67;    // 低8位初值
    ET0 = 1;       // 使能定时器0中断
    TR0 = 1;       // 启动定时器0
    EA = 1;        // 开启总中断
}

/**
 * @brief 动态扫描显示函数：交替点亮两个数码管
 */
void display() {
    unsigned char ten = num / 10;    // 十位数字（0~9）
    unsigned char unit = num % 10;   // 个位数字（0~9）

    // 显示十位（第二个数码管）
    SEG_PORT = seg_code[ten];
    BIT2 = 1;          // 点亮十位数码管
    BIT1 = 0;          // 熄灭个位数码管
    delay_ms(1);       // 延时1ms（保证亮度，避免闪烁）

    // 显示个位（第一个数码管）
    SEG_PORT = seg_code[unit];
    BIT1 = 1;          // 点亮个位数码管
    BIT2 = 0;          // 熄灭十位数码管
    delay_ms(1);       // 延时1ms
}

/**
 * @brief 毫秒级延时函数（简单延时，非精准）
 * @param ms 延时毫秒数
 */
void delay_ms(unsigned int ms) {
    unsigned int i, j;
    for (i = ms; i > 0; i--)
        for (j = 110; j > 0; j--);
}

/**
 * @brief 定时器0中断服务函数：1ms触发一次，累计1秒更新数字
 */
void timer0_isr() interrupt 1 {
    // 重新加载初值（保证每次中断间隔1ms）
    TH0 = 0xFC;
    TL0 = 0x67;
    count++;
    if (count >= 1000) {  // 累计1000ms = 1秒
        count = 0;        // 计数清零
        num++;            // 数字递增
        if (num >= 100) { // 超过99，重置为0
            num = 0;
        }
    }
}

/**
 * @brief 主函数：循环显示
 */
void main() {
    timer0_init();  // 初始化定时器0
    while (1) {
        display();  // 持续动态扫描显示
    }
}