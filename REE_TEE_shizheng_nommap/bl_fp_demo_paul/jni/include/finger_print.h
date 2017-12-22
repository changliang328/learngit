#ifndef __FINGER_PRINT_H
#define	__FINGER_PRINT_H

#ifdef __cplusplus
    #define LIB_API extern "C" 
#else
    #define LIB_API 
#endif
#define VERSION         32 //����ʱ�� 2017-12-05

enum FP_ERR_CODE
{
	FPEC_OK,                      //�ɹ�
	FPEC_ERR_OUTOFMEMORY,         //�ڴ治��
	FPEC_ERR_NULLREFERENCE,       //��ָ��
	FPEC_ERR_ARGUMENTOUTOFRANGE,  //��������ȷ
	FPEC_ERR_MOREENROLLNUM,       //¼�������������
	FPEC_ERR_LITTLEAREA,          //��Ч�����С
	FPEC_ERR_BADIMAGE,            //ͼ����������
	FPEC_ERR_LITTLEFEATURE,       //��������
	FPEC_ERR_DECODEFEATURE,       //ģ�����ʧ��
	FPEC_ERR_TRYCOUNTLIMIT,       //�������ô���
	FPEC_ERR_UNKNOWN              //δ֪����
};

/****************************
ͼ��Ԥ�����һ��,����ͼ��������������֮ǰ
������
imgData    [in][out]ͼ������
width      [in]ͼ���
height     [in]ͼ���
bgValue    [in]�������صĻҶ�ֵ,����0�����㷨�Զ�
pMinValidArea [in][out] ������С��Ч����ٷֱ�[0,100]�����ʵ����Ч���
nRepairFlag [in]�߿��޸���־λ
bit[0] �ϱ߿��޲���־��1��ʾ�޲���0��ʾ���޲�
bit[1] �±߿��޲���־��1��ʾ�޲���0��ʾ���޲�
bit[2] ��߿��޲���־��1��ʾ�޲���0��ʾ���޲�
bit[3] �ұ߿��޲���־��1��ʾ�޲���0��ʾ���޲�
����ֵ��
������
****************************/
LIB_API int FP_ImagePreproc_Step1(unsigned char* imgData, int width, int height, unsigned char bgValue,
    unsigned char* pMinValidArea, unsigned char nRepairFlag);

/****************************
ͼ��Ԥ����ڶ�����ʹ�þ�����һ��֮���ͼ�񣬷���ͼ��������������֮��
������
imgData    [in][out]����ͼ��Ԥ�����һ��֮���ͼ������
width      [in]ͼ���
height     [in]ͼ���
����ֵ��
������
****************************/
LIB_API int FP_ImagePreproc_Step2(unsigned char* imgData, int width, int height);

/****************************
ͼ��Ԥ������ͬ��FP_ImagePreproc_Step1+FP_ImagePreproc_Step2
������
imgData    [in][out]ͼ������
width      [in]ͼ���
height     [in]ͼ���
bgValue    [in]�������صĻҶ�ֵ,����0�����㷨�Զ�
pMinValidArea [in][out] ������С��Ч����ٷֱ�[0,100]�����ʵ����Ч���
nRepairFlag [in]�߿��޸���־λ
bit[0] �ϱ߿��޲���־��1��ʾ�޲���0��ʾ���޲�
bit[1] �±߿��޲���־��1��ʾ�޲���0��ʾ���޲�
bit[2] ��߿��޲���־��1��ʾ�޲���0��ʾ���޲�
bit[3] �ұ߿��޲���־��1��ʾ�޲���0��ʾ���޲�
����ֵ��
������
****************************/
int FP_ImagePreproc(unsigned char * imgData, int width, int height, unsigned char bgValue, unsigned char* pMinValidArea, unsigned char nRepairFlag);

/****************************
ͼ��Ԥ����,MCU��ʹ��,����ͼ����������֮ǰ
������
imgData    [in][out]ͼ������
width      [in]ͼ���
height     [in]ͼ���
bgValue    [in]�������صĻҶ�ֵ,����0�����㷨�Զ�
pMinValidArea [in][out] ������С��Ч����ٷֱ�[0,100]�����ʵ����Ч���
nRepairFlag [in]�߿��޸���־λ
bit[0] �ϱ߿��޲���־��1��ʾ�޲���0��ʾ���޲�
bit[1] �±߿��޲���־��1��ʾ�޲���0��ʾ���޲�
bit[2] ��߿��޲���־��1��ʾ�޲���0��ʾ���޲�
bit[3] �ұ߿��޲���־��1��ʾ�޲���0��ʾ���޲�
����ֵ��
������
****************************/
int FP_ImagePreproc_MCU(unsigned char * imgData, int width, int height, unsigned char bgValue, unsigned char* pMinValidArea, unsigned char nRepairFlag);

/****************************
��ȡ��İ汾��
������
��
����ֵ��
�汾��(��������10��ʾ1.0�汾)
****************************/
LIB_API int FP_GetVersion(void);

/****************************
��ȡָ������ľ�ֵ����������ģ�����FP_ImagePreproc_Step1������ͼ��
������
srcImg         [in]ָ��ͼ�񣨾�FP_ImagePreproc_Step1������ͼ��
width          [in]ͼ���
height         [in]ͼ���
nMean          [out]ָ������ľ�ֵ,��Χ[0,255]
nArea          [out]ָ����������ռ�ȣ���Χ[0,100]
nCenterX       [out]ָ�����������X���꣬��Χ[0,width-1]
nCenterY       [out]ָ�����������Y���꣬��Χ[0,height-1]
����ֵ��
������
****************************/
LIB_API int FP_GetFingerprintInfo(unsigned char * imgData, int width, int height, \
    unsigned char* nMean, unsigned char* nArea, int* nCenterX, int* nCenterY);

/**********************************
��ȡͼ������������ʹ�þ�FP_ImagePreproc_Step1������ͼ��
������
pImgData          [in]Դͼ(FP_ImagePreproc_Step1������ͼ��)
nWidth            [in]Դͼ���
nHeight           [in]Դͼ�߶�
nQuality          [out]����ͼ����������Χ[0,100]
nArea             [out]������Ч�������Χ[0,100]
nConditon         [out]���ظ�ʪ�̶ȣ���Χ[0,100]��50Ϊ��ѣ����±�ʾ��ʪ�������ʾ�ϸ�
nValidAreaMean    [out]����ͼ��ָ������ĻҶȾ�ֵ����Χ[0,255]
**********************************/
LIB_API int FP_ImageQuality(unsigned char * pImgData, int nWidth, int nHeight, unsigned char *nQuality, unsigned char *nArea, unsigned char *nConditon, unsigned char *nValidAreaMean);
/**********************************
��ȡͼ������, �����١���ʡ�ڴ棬�ʺ���MCU������
������
pImgData          [in]Դͼ(��FP_ImagePreproc_MCUԤ������ͼ��)
nWidth            [in]Դͼ���
nHeight           [in]Դͼ�߶�
nQuality          [out]����ͼ����������Χ[0,100]
nArea             [out]������Ч�������Χ[0,100]
nConditon         [out]���ظ�ʪ�̶ȣ���Χ[0,100]��50Ϊ��ѣ����±�ʾ��ʪ�������ʾ�ϸ�
nValidAreaMean    [out]����ͼ��ָ������ĻҶȾ�ֵ����Χ[0,255]
**********************************/
LIB_API int FP_ImageQuality_MCU(unsigned char * pImgData, int nWidth, int nHeight, unsigned char *nQuality, unsigned char *nArea, unsigned char *nConditon, unsigned char *nValidAreaMean);

/**********************************
���ͼ�����쳣���أ����ۡ����ƣ��ĸ����Ƿ񳬳��趨ֵ����API��ָ��ע���ģ�嶯̬����ʱʹ��
������
pImgData          [in]Դͼ(Ԥ����ǰ��ͼ��)
nWidth            [in]Դͼ���
nHeight           [in]Դͼ�߶�
nBadPixelMaxValue [in]�쳣���ص����Ҷ�ֵ������ֵ��COATINGģ�����128���ǰ�ģ�����10
nBadPixelsNumThre [in]ͼ���쳣���صĸ�����ֵ����BF3290��BF3180���140���������100
nCheckResult      [out]����ֵΪ1��ʾ�쳣���صĸ���������ֵ��0��ʾδ������ֵ
**********************************/
LIB_API int FP_CheckBadPixelsNum(const unsigned char * pImgData, int nWidth, int nHeight, unsigned char nBadPixelMaxValue, int nBadPixelsNumThre, char* nCheckResult);

/**********************************
��ȡͼ�����쳣���أ����ۡ����ƣ��ĸ�����������
������
pImgData          [in]Դͼ(Ԥ����ǰ��ͼ��)
nWidth            [in]Դͼ���
nHeight           [in]Դͼ�߶�
nBadPixelMaxValue [in]�쳣���ص����Ҷ�ֵ������ֵ��COATINGģ�����128���ǰ�ģ�����10
nBadPixelsNum     [out]����ͼ���쳣���صĸ���
**********************************/
LIB_API int FP_GetBadPixelsNum(const unsigned char * pImgData, int nWidth, int nHeight, unsigned char nBadPixelMaxValue, int* nBadPixelsNum);

/**********************************
���ͼ�����쳣���أ����ۡ����ƣ��ĸ����Ƿ񳬳��趨ֵ����API��ָ��ע���ģ�嶯̬����ʱʹ��,������MCU������
������
pImgData          [in]Դͼ(Ԥ����ǰ��ͼ��)
nWidth            [in]Դͼ���
nHeight           [in]Դͼ�߶�
nBadPixelMaxValue [in]�쳣���ص����Ҷ�ֵ������ֵ��COATINGģ�����128���ǰ�ģ�����10
nBadPixelsNumThre [in]ͼ���쳣���صĸ�����ֵ��BF3290��BF3180���140���������100
nCheckResult      [out]����ֵΪ1��ʾ�쳣���صĸ���������ֵ��0��ʾδ������ֵ
**********************************/
LIB_API int FP_CheckBadPixelsNum_MCU(const unsigned char * pImgData, int nWidth, int nHeight, unsigned char nBadPixelMaxValue, int nBadPixelsNumThre, char* nCheckResult);

////////////////////////////////////////////����Ϊ������////////////////////////////////////////
#endif
