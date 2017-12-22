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
��AGC��صļĴ���
����dacp�����Ĵ�����Ӧ��ϵ���£�
BF3290��BF2390E ���� 0x1b
BF3182          ���� 0x1c
BF3390          ���� 0x1e
BF3590          ���� 0x1c
*/
typedef struct tag_REG_VALUE
{
	unsigned char reg_value_0x31; //�Ĵ���0x31��ֵ
	unsigned char reg_value_0x32; //�Ĵ���0x32��ֵ
	unsigned char reg_value_dacp; //dacp��ֵ
}REG_VALUE, *PREG_VALUE;

/******************************************************************************
AGC������ʹ��ͼ������
imgdata          [in]ͼ�����ݣ�������ROIͼ���ɼ�ͼ���м䲿�֣�
width            [in]ͼ���
height           [in]ͼ���
pOldMean         [in]ͼ����Ч����ĻҶȾ�ֵ�����û�У������ָ�뼴��
oldRegValue      [in]���ڵõ���ͼ��ļĴ���ֵ
newRegValue      [out]����µļĴ���ֵ��Ŀǰֻ��dacp��ı�
dstMean          [in]ͼ��Ŀ��ҶȾ�ֵ
nSensorType      [in]ģ�����ͣ�ST_PASSIVE��ʾ����ʽ����3290��3182��3390�ȣ���ST_ACTIVE��ʾ����ʽ����2390E��2180�ȣ���ST_JACK��ʾBF3590��BF3582��
******************************************************************************/
LIB_API int bl_AutoGainDacp(unsigned char* imgdata, int width, int height, unsigned char* pOldMean, \
    REG_VALUE* oldRegValue, REG_VALUE* newRegValue, unsigned char dstMean, int nSensorType);

#endif