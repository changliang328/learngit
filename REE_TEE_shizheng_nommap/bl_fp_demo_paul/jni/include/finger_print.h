#ifndef __FINGER_PRINT_H
#define	__FINGER_PRINT_H

#ifdef __cplusplus
    #define LIB_API extern "C" 
#else
    #define LIB_API 
#endif
#define VERSION         32 //更改时间 2017-12-05

enum FP_ERR_CODE
{
	FPEC_OK,                      //成功
	FPEC_ERR_OUTOFMEMORY,         //内存不足
	FPEC_ERR_NULLREFERENCE,       //空指针
	FPEC_ERR_ARGUMENTOUTOFRANGE,  //参数不正确
	FPEC_ERR_MOREENROLLNUM,       //录入次数超出设置
	FPEC_ERR_LITTLEAREA,          //有效面积过小
	FPEC_ERR_BADIMAGE,            //图像质量过差
	FPEC_ERR_LITTLEFEATURE,       //特征过少
	FPEC_ERR_DECODEFEATURE,       //模板解码失败
	FPEC_ERR_TRYCOUNTLIMIT,       //超出试用次数
	FPEC_ERR_UNKNOWN              //未知错误
};

/****************************
图像预处理第一步,放在图像质量评估函数之前
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
LIB_API int FP_ImagePreproc_Step1(unsigned char* imgData, int width, int height, unsigned char bgValue,
    unsigned char* pMinValidArea, unsigned char nRepairFlag);

/****************************
图像预处理第二步，使用经过第一步之后的图像，放在图像质量评估函数之后
参数：
imgData    [in][out]经过图像预处理第一步之后的图像数据
width      [in]图像宽
height     [in]图像高
返回值：
错误码
****************************/
LIB_API int FP_ImagePreproc_Step2(unsigned char* imgData, int width, int height);

/****************************
图像预处理，等同于FP_ImagePreproc_Step1+FP_ImagePreproc_Step2
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
int FP_ImagePreproc(unsigned char * imgData, int width, int height, unsigned char bgValue, unsigned char* pMinValidArea, unsigned char nRepairFlag);

/****************************
图像预处理,MCU端使用,放在图像质量评估之前
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
int FP_ImagePreproc_MCU(unsigned char * imgData, int width, int height, unsigned char bgValue, unsigned char* pMinValidArea, unsigned char nRepairFlag);

/****************************
获取库的版本号
参数：
无
返回值：
版本号(整数，如10表示1.0版本)
****************************/
LIB_API int FP_GetVersion(void);

/****************************
获取指纹区域的均值、面积和质心，基于FP_ImagePreproc_Step1处理后的图像
参数：
srcImg         [in]指纹图像（经FP_ImagePreproc_Step1处理后的图像）
width          [in]图像宽
height         [in]图像高
nMean          [out]指纹区域的均值,范围[0,255]
nArea          [out]指纹区域的面积占比，范围[0,100]
nCenterX       [out]指纹区域的质心X坐标，范围[0,width-1]
nCenterY       [out]指纹区域的质心Y坐标，范围[0,height-1]
返回值：
错误码
****************************/
LIB_API int FP_GetFingerprintInfo(unsigned char * imgData, int width, int height, \
    unsigned char* nMean, unsigned char* nArea, int* nCenterX, int* nCenterY);

/**********************************
获取图像质量，必须使用经FP_ImagePreproc_Step1处理后的图像
参数：
pImgData          [in]源图(FP_ImagePreproc_Step1处理后的图像)
nWidth            [in]源图宽度
nHeight           [in]源图高度
nQuality          [out]返回图像质量，范围[0,100]
nArea             [out]返回有效面积，范围[0,100]
nConditon         [out]返回干湿程度，范围[0,100]，50为最佳，往下表示较湿，往大表示较干
nValidAreaMean    [out]返回图像指纹区域的灰度均值，范围[0,255]
**********************************/
LIB_API int FP_ImageQuality(unsigned char * pImgData, int nWidth, int nHeight, unsigned char *nQuality, unsigned char *nArea, unsigned char *nConditon, unsigned char *nValidAreaMean);
/**********************************
获取图像质量, 更快速、更省内存，适合在MCU中运行
参数：
pImgData          [in]源图(经FP_ImagePreproc_MCU预处理后的图像)
nWidth            [in]源图宽度
nHeight           [in]源图高度
nQuality          [out]返回图像质量，范围[0,100]
nArea             [out]返回有效面积，范围[0,100]
nConditon         [out]返回干湿程度，范围[0,100]，50为最佳，往下表示较湿，往大表示较干
nValidAreaMean    [out]返回图像指纹区域的灰度均值，范围[0,255]
**********************************/
LIB_API int FP_ImageQuality_MCU(unsigned char * pImgData, int nWidth, int nHeight, unsigned char *nQuality, unsigned char *nArea, unsigned char *nConditon, unsigned char *nValidAreaMean);

/**********************************
检查图像中异常像素（划痕、裂纹）的个数是否超出设定值，此API在指纹注册和模板动态更新时使用
参数：
pImgData          [in]源图(预处理前的图像)
nWidth            [in]源图宽度
nHeight           [in]源图高度
nBadPixelMaxValue [in]异常像素的最大灰度值，经验值，COATING模组设成128，盖板模组设成10
nBadPixelsNumThre [in]图像异常像素的个数阈值，，BF3290和BF3180设成140，其它设成100
nCheckResult      [out]返回值为1表示异常像素的个数超出阈值，0表示未超出阈值
**********************************/
LIB_API int FP_CheckBadPixelsNum(const unsigned char * pImgData, int nWidth, int nHeight, unsigned char nBadPixelMaxValue, int nBadPixelsNumThre, char* nCheckResult);

/**********************************
获取图像中异常像素（划痕、裂纹）的个数，调试用
参数：
pImgData          [in]源图(预处理前的图像)
nWidth            [in]源图宽度
nHeight           [in]源图高度
nBadPixelMaxValue [in]异常像素的最大灰度值，经验值，COATING模组设成128，盖板模组设成10
nBadPixelsNum     [out]返回图像异常像素的个数
**********************************/
LIB_API int FP_GetBadPixelsNum(const unsigned char * pImgData, int nWidth, int nHeight, unsigned char nBadPixelMaxValue, int* nBadPixelsNum);

/**********************************
检查图像中异常像素（划痕、裂纹）的个数是否超出设定值，此API在指纹注册和模板动态更新时使用,适用于MCU中运行
参数：
pImgData          [in]源图(预处理前的图像)
nWidth            [in]源图宽度
nHeight           [in]源图高度
nBadPixelMaxValue [in]异常像素的最大灰度值，经验值，COATING模组设成128，盖板模组设成10
nBadPixelsNumThre [in]图像异常像素的个数阈值，BF3290和BF3180设成140，其它设成100
nCheckResult      [out]返回值为1表示异常像素的个数超出阈值，0表示未超出阈值
**********************************/
LIB_API int FP_CheckBadPixelsNum_MCU(const unsigned char * pImgData, int nWidth, int nHeight, unsigned char nBadPixelMaxValue, int nBadPixelsNumThre, char* nCheckResult);

////////////////////////////////////////////以下为调试用////////////////////////////////////////
#endif
