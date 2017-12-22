#ifndef __AUTO_GAIN_DACP_H
#define	__AUTO_GAIN_DACP_H

#ifdef __cplusplus
    #define LIB_API extern "C" 
#else
    #define LIB_API 
#endif

enum SENSOR_TYPE
{
	ST_PASSIVE,
	ST_ACTIVE,
    ST_JACK,         //BF3590
};

/*
与AGC相关的寄存器
其中dacp与具体寄存器对应关系如下：
BF3290、BF2390E —— 0x1b
BF3182          —— 0x1c
BF3390          —— 0x1e
BF3590          —— 0x1c
*/
typedef struct tag_REG_VALUE
{
	unsigned char reg_value_0x31; //寄存器0x31的值
	unsigned char reg_value_0x32; //寄存器0x32的值
	unsigned char reg_value_dacp; //dacp的值
}REG_VALUE, *PREG_VALUE;

/******************************************************************************
AGC函数，使用图像数据
imgdata          [in]图像数据（可以是ROI图，采集图像中间部分）
width            [in]图像宽
height           [in]图像高
pOldMean         [in]图像有效区域的灰度均值，如果没有，传入空指针即可
oldRegValue      [in]用于得到此图像的寄存器值
newRegValue      [out]输出新的寄存器值，目前只有dacp会改变
dstMean          [in]图像目标灰度均值
nSensorType      [in]模组类型，ST_PASSIVE表示被动式（如3290、3182、3390等），ST_ACTIVE表示主动式（如2390E、2180等），ST_JACK表示BF3590、BF3582。
******************************************************************************/
LIB_API int bl_AutoGainDacp(unsigned char* imgdata, int width, int height, unsigned char* pOldMean, \
    REG_VALUE* oldRegValue, REG_VALUE* newRegValue, unsigned char dstMean, int nSensorType);

#endif