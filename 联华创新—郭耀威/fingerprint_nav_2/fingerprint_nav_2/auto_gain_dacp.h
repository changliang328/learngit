#ifndef __AUTO_GAIN_DACP_H
#define	__AUTO_GAIN_DACP_H

#define LIB_API //extern "C" 

typedef struct tag_REG_VALUE_3290
{
	unsigned char reg_value_0x31; //�Ĵ���0x31��ֵ
	unsigned char reg_value_0x32; //�Ĵ���0x32��ֵ
	unsigned char reg_value_0x1b; //�Ĵ���0x1b��ֵ
}REG_VALUE_3290, *PREG_VALUE_3290;

typedef struct tag_REG_VALUE_3182
{
	unsigned char reg_value_0x31; //�Ĵ���0x31��ֵ
	unsigned char reg_value_0x32; //�Ĵ���0x32��ֵ
	unsigned char reg_value_0x1c; //�Ĵ���0x1c��ֵ
}REG_VALUE_3182, *PREG_VALUE_3182;

typedef struct tag_GAIN_DACP
{
	unsigned char reg_value_gain1;
	unsigned char reg_value_gain2;
	unsigned char reg_value_gain3;
	unsigned char reg_value_gain4;
	unsigned char reg_value_dacp;
}GAIN_DACP, *PGAIN_DACP;

LIB_API int bl_AutoGainDacp_3290(unsigned char* imgdata, int width, int height, PREG_VALUE_3290 oldRegValue, PREG_VALUE_3290 newRegValue, unsigned char dstMean);
LIB_API int bl_AutoGainDacp_3182(unsigned char* imgdata, int width, int height, PREG_VALUE_3182 oldRegValue, PREG_VALUE_3182 newRegValue, unsigned char dstMean);
LIB_API int bl_AutoGainDacp(unsigned char* imgdata, int nWidth, int nHeight, PGAIN_DACP oldGainDacp, PGAIN_DACP newGainDacp, unsigned char dstMean);


#endif
