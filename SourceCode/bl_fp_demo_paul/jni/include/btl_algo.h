#ifndef __BTL_ALGO_H
#define	__BTL_ALGO_H

#ifdef __cplusplus
    #define BTL_ALGO_EXT extern "C"
#else
	#define BTL_ALGO_EXT 
#endif

#define BL_ALG_VERSION 110
/*
������
*/
enum FPALG_ERR_CODE
{
	FP_EC_OK,                      // �ɹ�
	FP_EC_ERR_OUTOFMEMORY,         // �ڴ治��
	FP_EC_ERR_NULLREFERENCE,       // ��ָ��
	FP_EC_ERR_WRONG_PARAM,         // �����������ȷ
	FP_EC_ERR_EMPTY_TEMPLATE,      //ģ��Ϊ��
	FP_EC_ERR_CREATE_IMG,          //ͼ�񴴽�ʧ��
	FP_EC_ERR_CREATE_TEMPLATE,     //ģ�崴��ʧ��
	FP_EC_ERR_EXTRACT_TEMPLATE,    //��ȡģ��ʧ��
	FP_EC_ERR_UNKNOWN              // δ֪����
};

/*
�㷨����
*/
typedef enum algorithm_type_e
{
	AT_NEO_HENRY,                   // neo_bf3290
	AT_NEO_HENRY_SPEED,             // neo_bf3290_speed
	AT_NEO_GALTON,                  // neo_3182
	AT_NEO_GALTON_SPEED,            // neo_3182_speed
	AT_NEO_SQUARE_M,                // 130x130 pixels @ 500 dpi    
	AT_NEO_SQUARE_M_SPEED,
	AT_NEO_SQUARE_S,                // 115x115 pixels @ 500 dpi
	AT_NEO_SQUARE_S_SPEED,
	AT_NEO_SQUARE_XS,               // 100x100 pixels @ 500 dpi
	AT_NEO_SQUARE_XS_SPEED,
	AT_NEO_SQUARE_XXS,              // 80*80
	AT_NEO_RECTANGULAR_M,           // 200x80 pixels @ 500 dpi
	AT_NEO_RECTANGULAR_M_SPEED,
	AT_NEO_RECTANGULAR_S,           // 200x60 pixels @ 500 dpi
	AT_NEO_RECTANGULAR_S_SPEED,
	AT_HYBRID_SQUARE_XSS_NON_HR,    // 80*80
	AT_HYBRID_SQUARE_XS_SPEED_MEM,  // MCU 100*100
	AT_HYBRID_SQUARE_S_SPEED_MEM,   // MCU
	AT_HYBRID_SQUARE_M_SPEED_MEM,   // MCU
	AT_HYBRID_SQUARE_L_SPEED_MEM,   // MCU 160*160
	AT_HYBRID_SQUARE_XL_SPEED_MEM,  // MCU 200*200
	AT_HYBRID_HENRY,                // ehm_nonhr_bf3290
	AT_HYBRID_GALTON,               // ehm_nonhr_bf3182
	AT_CARDO_HENRY,                 // cardo_bf3290
	AT_CARDO_GALTON,                // cardo_bf3182
	AT_CARDO_SQUARE_XXS,            // 80x80 pixels @ 500 dpi
	AT_CARDO_SQUARE_XS,             // 100x100 pixels @ 500 dpi
	AT_CARDO_RECTANGULAR_S,         // 200x60
	AT_SPECTRAL_SQUARE_S,           // 115*115
	AT_END
}algorithm_type_t;

typedef enum bl_far_e
{
	BL_FAR_1 = 0,
	BL_FAR_2 = 1,
	BL_FAR_5 = 2,
	BL_FAR_10 = 3,
	BL_FAR_20 = 4,
	BL_FAR_50 = 5,
	BL_FAR_100 = 6,
	BL_FAR_200 = 7,
	BL_FAR_500 = 8,
	BL_FAR_1000 = 9,
	BL_FAR_2K = 10,
	BL_FAR_5000 = 11,
	BL_FAR_10000 = 12,
	BL_FAR_20K = 13,
	BL_FAR_50000 = 14,
	BL_FAR_100000 = 15,
	BL_FAR_200K = 16,
	BL_FAR_500000 = 17,
	BL_FAR_1000000 = 18,
	BL_FAR_2M = 19,
	BL_FAR_5M = 20,
	BL_FAR_10M = 21,
	BL_FAR_20M = 22,
	BL_FAR_50M = 23,
	BL_FAR_100M = 24,
	BL_FAR_200M = 25,
	BL_FAR_500M = 26,
	BL_FAR_1000M = 27,
	BL_FAR_Inf = 28
} bl_far_t;

typedef struct BL_INIT_PARAM_tag
{
	algorithm_type_t nAlgorithmType;      // �㷨����ѡ��
	unsigned int nMaxTemplateSize;        // ��ģ������ֽ���
	unsigned char nMaxNbrofSubtemplates;  // ��ģ��������ģ��������
	unsigned char bSupport360Rotate;      // �Ƿ�֧��360��ƥ��, 1��ʾ֧�֣�0��ʾ��֧��
}BL_INIT_PARAM, *PBL_INIT_PARAM;

/*
ָ��ģ��ṹ��
*/
typedef struct BL_TEMPLATE_tag
{
	unsigned char* pTemplateData;      //ģ������
	unsigned int  templateType;       //ģ������
	unsigned int   templateSize;       //ģ�����ݵĴ�С
}BL_TEMPLATE, *PBL_TEMPLATE;


/*
��ȡ��İ汾��
*/	
BTL_ALGO_EXT int  bl_Alg_GetVersion(
    int* pVersion                 //[out] �����㷨��İ汾��
    );

/*
��ʼ��
*/	
BTL_ALGO_EXT int  bl_Alg_Init(
    unsigned char nAlgID,          //[in] ָ���㷨ID
    const BL_INIT_PARAM* pParam    //[in] �㷨����
    );
/*
�ͷ���Դ
*/	
BTL_ALGO_EXT int  bl_Alg_UnInit(
    unsigned char nAlgID           //[in] ָ���㷨ID
    );

/*
ͼ�������ж�
*/
BTL_ALGO_EXT int  bl_Alg_ImageQuality(
        const unsigned char* imgdata,  //[in] ָ��ͼ������
		int width,                     //[in] ͼ���BF3290Ϊ112
		int height,                    //[in] ͼ��ߣ�BF3290Ϊ96
		unsigned char* pQuality,       //[out] ����ͼ����������Χ[0,100]
		unsigned char* pArea,          //[out] ����ͼ����Ч�������Χ[0,100]
		unsigned char* pCondition      //[out] ����ͼ���ʪ�̶ȣ���Χ[0,100]
		);

/*
ָ��¼��
*/
BTL_ALGO_EXT int  bl_Alg_Enroll(
    unsigned char nAlgID,               //[in] ָ���㷨ID
    const unsigned char* imgdata,       //[in] ָ��ͼ������
    int width,                          //[in] ͼ���BF3290Ϊ112
    int height,                         //[in] ͼ��ߣ�BF3290Ϊ96
    const PBL_TEMPLATE pblTemplate,     //[in] ��ǰ��ȡ��ָ��ģ�壬�������ģ����Լ���¼�룬���������0
    int* pAcceptedNum,                  //[out] �ѳɹ�¼��ģ����
    unsigned char* pCoverage,           //[out] ָ��¼�븲���������Χ[0,100]
    unsigned char* pNbrOfIslands        //[out] �µ�����
    );

/*
����ָ��¼�룬������ģ��
*/			
BTL_ALGO_EXT int  bl_Alg_FinalizeEnroll(
    unsigned char nAlgID,                   //[in] ָ���㷨ID
    PBL_TEMPLATE pblMultiTemplates        //[out] �����Ѿ����ɵ�ģ�����ݣ�֧��Ԥ�ȷ��������ڴ�
    );
/*
ȡ��ָ��¼��
*/	
BTL_ALGO_EXT int  bl_Alg_CancelEnroll(
    unsigned char nAlgID                   //[in] ָ���㷨ID
    );

/*
ָ����֤��ʹ��ͼ��
*/	
BTL_ALGO_EXT int  bl_Alg_Verify(
    unsigned char nAlgID,                   //[in] ָ���㷨ID
    const unsigned char* imgdata,           //[in] ָ��ͼ������
	int width,                              //[in] ͼ���
	int height,                             //[in] ͼ���
    const PBL_TEMPLATE* pblMultiTemplates,  //[in] ��¼��ģ������ 
	unsigned char nNbrOfMultiTemplates,     //[in] ��¼��ģ������	
    int nFarAccepted,                       //[in] FAR
	int* pMatchIndex                        //[out] ƥ��ɹ���ģ����ţ���0��ʼ��-1��ʾƥ��ʧ��
	);

/*
ָ����֤��ʹ��ģ��
*/	
BTL_ALGO_EXT int  bl_Alg_VerifyT(
    unsigned char nAlgID,                   //[in] ָ���㷨ID
    const PBL_TEMPLATE pblTemplate,         //[in] ����ָ֤��ģ��
    const PBL_TEMPLATE* pblMultiTemplates,  //[in] ��¼���ģ������ 
	unsigned char nNbrOfMultiTemplates,     //[in] ��¼��ģ������	
	int nFarAccepted,                       //[in] FAR
	int* pMatchIndex                        //[out] ƥ��ɹ���ģ����ţ���0��ʼ��-1��ʾƥ��ʧ��
	);

/*
ָ��ģ���ȡ
*/
BTL_ALGO_EXT int  bl_Alg_ExtractTemplate(
    unsigned char nAlgID,                  //[in] ָ���㷨ID
	unsigned char* imgdata,                //[in] ָ��ͼ������
	int width,                             //[in] ͼ���
	int height,                            //[in] ͼ���
	PBL_TEMPLATE pblTemplate             //[out] ��ȡ����ģ�����ݣ�֧��Ԥ�ȷ��������ڴ�
	);	

/*
ģ�嶯̬����
*/
BTL_ALGO_EXT int bl_Alg_UpdateMutilTemplates(
    unsigned char nAlgID,                   //[in] ָ���㷨ID
    const PBL_TEMPLATE pblTemplate,         //[in] ��ǰָ��ģ��
    const PBL_TEMPLATE pblMultiTemplates,   //[in] �����¶�ģ��
    PBL_TEMPLATE pblUpdateMultiTemplates    //[out] ���º��ģ�壬֧��Ԥ�ȷ��������ڴ�
	);

/*
ɾ��ģ��
*/
BTL_ALGO_EXT int bl_Alg_DeleteTemplate(
    PBL_TEMPLATE pblTemplate
    );

/*
ͼ����ǿ
*/
BTL_ALGO_EXT int bl_Alg_Enhance(
    unsigned char* imgdata,               //[in] ָ��ͼ������
    int width,                            //[in] ͼ���
    int height                            //[in] ͼ���
    );

#endif


		
