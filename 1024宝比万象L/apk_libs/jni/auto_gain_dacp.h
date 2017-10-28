#ifndef __AUTO_GAIN_DACP_H
#define	__AUTO_GAIN_DACP_H

#define LIB_API //extern "C" 

/*
��AGC��صļĴ���
����dacp�����Ĵ�����Ӧ��ϵ���£�
BF3290��BF2390E ���� 0x1b
BF3182          ���� 0x1c
BF3390          ���� 0x1e
*/
typedef struct tag_REG_VALUE
{
	unsigned char reg_value_0x31; //�Ĵ���0x31��ֵ
	unsigned char reg_value_0x32; //�Ĵ���0x32��ֵ
	unsigned char reg_value_dacp; //dacp��ֵ
}REG_VALUE, *PREG_VALUE;

/*
AGC����
imgdata  [in]ͼ�����ݣ�������ROIͼ���ɼ�ͼ���м䲿�֣�
width    [in]ͼ���
height   [in]ͼ���
oldRegValue  [in]���ڵõ���ͼ��ļĴ���ֵ
newRegValue  [out]����µļĴ���ֵ��Ŀǰֻ��dacp��ı�
dstMean    [in]ͼ��Ŀ��ҶȾ�ֵ
bActive    [in]�Ƿ�����ʽоƬ��1��ʾ�ǣ�0��ʾ���ǣ�����2390EӦ������Ϊ1
*/
LIB_API int bl_AutoGainDacp(unsigned char* imgdata, int width, int height, REG_VALUE* oldRegValue, REG_VALUE* newRegValue, unsigned char dstMean, char bActive);


#endif
