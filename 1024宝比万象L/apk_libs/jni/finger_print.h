#ifndef __FINGER_PRINT_H
#define	__FINGER_PRINT_H

//#define LIB_API extern "C"
#define LIB_API
#define VERSION         19 //版本号1.8

/****************************
图像预处理函数
参数：
imgData    [in][out]图像数据
width      [in]图像宽
height     [in]图像高
bgValue    [in]背景像素的灰度值,输入0则由算法自定
pMinValidArea [in][out] 输入最小有效面积百分比[0,100]，输出实际有效面积
nRepairFlag [in]边框修复标志位 
		bit[0] 上边框修补标志，1表示修补，0表示不修补
		bit[1] 下边框修补标志，1表示修补，0表示不修补
		bit[2] 左边框修补标志，1表示修补，0表示不修补
		bit[3] 右边框修补标志，1表示修补，0表示不修补
返回值：
错误码
****************************/
LIB_API int FP_ImagePreproc(unsigned char* imgData, int width, int height, unsigned char bgValue,
	unsigned char* pMinValidArea, unsigned char nRepairFlag);

/****************************
获取库的版本号
参数：
无
返回值：
版本号(整数，如10表示1.0版本)
****************************/
LIB_API int FP_GetVersion();

/****************************
获取有效区域的百分比
参数：
srcImg         [in]指纹图像
width          [in]图像宽
height         [in]图像高
bgValue        [in]背景区域最小灰度值，如果传入0则由算法自定
返回值：
有效区域所占百分比[0,100]
****************************/
LIB_API unsigned char FP_GetValidAreaRatio(unsigned char * srcImg, int width, int height, unsigned char bgValue);
#endif