/*============================================================
 * ADC0832.c  --  ADC0832 串行8位A/D转换器驱动实现
 *
 * 项目：基于STC89C52单片机的智能汽车雨刮器模拟系统
 * 作者：梁嘉惠  学号：3222004950
 * 学校：五邑大学 电子与信息工程学院  班级：220710
 * 指导教师：龙佳乐 副教授
 * 编译器：Keil C51  主频：11.0592MHz
 *============================================================
 * ADC0832通信时序说明（软件SPI）：
 *   1. CS拉低，开始通信
 *   2. 发送起始位(1) + SGL位(1，单端模式) + MUX位(0，选CH0)
 *      共3个控制位，在CLK上升沿采样
 *   3. 等待一个CLK后，从DO读取8位转换结果（MSB先出）
 *   4. CS拉高，结束通信
 * 转换结果：干燥时约255，完全浸水时约0
 *============================================================*/

#include "ADC0832.h"

/* ===== 滤波缓冲区（全局，供滤波函数使用） ===== */
#define FILTER_N  8
static unsigned char s_flt_buf[FILTER_N];
static unsigned char s_flt_idx = 0;
static unsigned char s_flt_init = 0;   /* 首次初始化标志 */

/*------------------------------------------------------------
 * ADC0832_Read_CH0()
 * 功能：读取ADC0832的CH0通道（单端输入）
 * 返回：8位转换结果（0~255）
 *       0   → 电压最低（约0V），对应大雨（传感器电阻最小）
 *       255 → 电压最高（约5V），对应无雨（传感器电阻最大）
 * 说明：完全依照ADC0832数据手册时序实现
 *------------------------------------------------------------*/
unsigned char ADC0832_Read_CH0(void)
{
    unsigned char i;
    unsigned char dat = 0;

    /* ① CS拉低，开始一次转换 */
    ADC_CS  = 0;
    ADC_CLK = 0;

    /* ② 发送起始位 = 1 */
    ADC_DI = 1;
    _nop_(); _nop_();
    ADC_CLK = 1;
    _nop_(); _nop_();
    ADC_CLK = 0;

    /* ③ 发送SGL = 1（单端模式，对应MUX选CH0/CH1） */
    ADC_DI = 1;
    _nop_(); _nop_();
    ADC_CLK = 1;
    _nop_(); _nop_();
    ADC_CLK = 0;

    /* ④ 发送ODD/SIGN = 0（选CH0为正，CH1为负，即读CH0） */
    ADC_DI = 0;
    _nop_(); _nop_();
    ADC_CLK = 1;
    _nop_(); _nop_();
    ADC_CLK = 0;

    /* ⑤ 释放DI，等待DO输出，此CLK为空CLK（转换完成后才输出） */
    ADC_DI = 1;
    _nop_(); _nop_();

    /* ⑥ 读取8位结果（MSB先出） */
    for (i = 0; i < 8; i++)
    {
        dat <<= 1;                  /* 先移位，再读位 */
        ADC_CLK = 1;
        _nop_(); _nop_();
        if (ADC_DO)
            dat |= 0x01;            /* 在时钟高期间读DO */
        ADC_CLK = 0;
        _nop_(); _nop_();
    }

    /* ⑦ CS拉高，结束通信 */
    ADC_CS = 1;

    return dat;
}

/*------------------------------------------------------------
 * ADC0832_Filter()
 * 功能：读取ADC0832并进行8点滑动平均滤波
 * 返回：滤波后的ADC值（0~255）
 * 说明：消除因电机PWM噪声和传感器抖动导致的数值跳变
 *       首次调用时用第一次采样值填满缓冲区，避免冷启动偏差
 *------------------------------------------------------------*/
unsigned char ADC0832_Filter(void)
{
    unsigned char i;
    unsigned char raw;
    unsigned int  sum = 0;

    raw = ADC0832_Read_CH0();

    /* 首次调用，用当前值填满整个缓冲区 */
    if (s_flt_init == 0)
    {
        s_flt_init = 1;
        for (i = 0; i < FILTER_N; i++)
            s_flt_buf[i] = raw;
        s_flt_idx = 0;
        return raw;
    }

    /* 滑动写入 */
    s_flt_buf[s_flt_idx] = raw;
    s_flt_idx = (s_flt_idx + 1) % FILTER_N;

    /* 累加求均值 */
    for (i = 0; i < FILTER_N; i++)
        sum += s_flt_buf[i];

    return (unsigned char)(sum / FILTER_N);
}
