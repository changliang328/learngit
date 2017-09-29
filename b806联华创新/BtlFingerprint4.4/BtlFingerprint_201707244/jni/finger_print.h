#ifndef __FINGER_PRINT_H
#define	__FINGER_PRINT_H

//#define LIB_API extern "C"
#define LIB_API
#define VERSION         19 //�汾��1.8

/****************************
ͼ��Ԥ������
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
LIB_API int FP_ImagePreproc(unsigned char* imgData, int width, int height, unsigned char bgValue,
	unsigned char* pMinValidArea, unsigned char nRepairFlag);

/****************************
��ȡ��İ汾��
������
��
����ֵ��
�汾��(��������10��ʾ1.0�汾)
****************************/
LIB_API int FP_GetVersion();

/****************************
��ȡ��Ч����İٷֱ�
������
srcImg         [in]ָ��ͼ��
width          [in]ͼ���
height         [in]ͼ���
bgValue        [in]����������С�Ҷ�ֵ���������0�����㷨�Զ�
����ֵ��
��Ч������ռ�ٷֱ�[0,100]
****************************/
LIB_API unsigned char FP_GetValidAreaRatio(unsigned char * srcImg, int width, int height, unsigned char bgValue);
#endif