#ifndef __AUTO_GAIN_DACP_H
#define	__AUTO_GAIN_DACP_H

#define LIB_API //extern "C" 

/*
与AGC相关的寄存器
其中dacp与具体寄存器对应关系如下：
BF3290、BF2390E —— 0x1b
BF3182          —— 0x1c
BF3390          —— 0x1e
*/
typedef struct tag_REG_VALUE
{
	unsigned char reg_value_0x31; //寄存器0x31的值
	unsigned char reg_value_0x32; //寄存器0x32的值
	unsigned char reg_value_dacp; //dacp的值
}REG_VALUE, *PREG_VALUE;

/*
AGC函数
imgdata  [in]图像数据（可以是ROI图，采集图像中间部分）
width    [in]图像宽
height   [in]图像高
oldRegValue  [in]用于得到此图像的寄存器值
newRegValue  [out]输出新的寄存器值，目前只有dacp会改变
dstMean    [in]图像目标灰度均值
bActive    [in]是否主动式芯片，1表示是，0表示不是，比如2390E应该设置为1
*/
LIB_API int bl_AutoGainDacp(unsigned char* imgdata, int width, int height, REG_VALUE* oldRegValue, REG_VALUE* newRegValue, unsigned char dstMean, char bActive);


#endif
