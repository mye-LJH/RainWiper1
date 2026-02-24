/*============================================================
 * oled.h  --  SSD1306 OLED I2C驱动头文件（软件模拟I2C）
 *
 * 项目：基于STC89C52单片机的智能汽车雨刮器模拟系统
 * 作者：梁嘉惠  学号：3222004950
 * 学校：五邑大学 电子与信息工程学院  班级：220710
 * 指导教师：龙佳乐 副教授
 * 编译器：Keil C51  主频：11.0592MHz
 *============================================================
 * 硬件连接（STC89C52 <-> SSD1306 OLED 4Pin I2C版）：
 *   P1.6  <->  SCL  (I2C时钟，需4.7kΩ上拉到VCC)  ★必须加上拉!
 *   P1.7  <->  SDA  (I2C数据，需4.7kΩ上拉到VCC)  ★必须加上拉!
 *   5V    <->  VCC  (3.3V~5V均可)
 *   GND   <->  GND
 * I2C地址：0x3C（默认）/ 0x3D（修改背面焊盘）
 *============================================================
 * OLED显示内容规划（128×64像素，共4行16列）：
 *   第1行 (y=0): "RainWiper V4.0" — 标题
 *   第2行 (y=2): "ADC:xxx"        — ADC原始值(0~255)
 *   第3行 (y=4): "Level:Xxx Rain" — 雨量等级
 *   第4行 (y=6): "PWM:xx% Xxxxx"  — 占空比+模式
 *============================================================*/

#ifndef __OLED_H__
#define __OLED_H__

#include <reg52.h>
#include <intrins.h>

/* ===== I2C引脚定义 ===== */
sbit OLED_SCL = P1^6;   /* I2C时钟线 SCL */
sbit OLED_SDA = P1^7;   /* I2C数据线 SDA */

/* ===== SSD1306 I2C地址（7位地址） ===== */
/* 若OLED点不亮，尝试改为 0x3D */
#define OLED_ADDR   0x3C

/* ===== OLED分辨率 ===== */
#define OLED_WIDTH  128
#define OLED_HEIGHT  64

/* ===== 函数声明 ===== */

/* 底层初始化与清屏 */
void OLED_Init(void);           /* OLED上电初始化（SSD1306标准序列） */
void OLED_Clear(void);          /* 清屏（全黑）                      */

/* 字符/字符串显示（8×16字体，y为page地址，0/2/4/6） */
void OLED_ShowChar(unsigned char x, unsigned char y, char c);
void OLED_ShowStr(unsigned char x, unsigned char y, char *str);

/* 数字显示 */
void OLED_ShowNum(unsigned char x, unsigned char y,
                  unsigned int num, unsigned char len);

/* 整屏内容刷新（从全局变量读取雨量数据后调用） */
void OLED_RefreshAll(unsigned char adc_val,
                     unsigned char rain_lv,
                     unsigned char pwm_duty,
                     unsigned char auto_mode);

/* 开机画面（含个人信息） */
void OLED_ShowBootScreen(void);

#endif /* __OLED_H__ */
