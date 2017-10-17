#ifndef __BTL_ALGO_H
#define	__BTL_ALGO_H

#ifdef __cplusplus
    #define BTL_ALGO_EXT extern "C"
#else
	#define BTL_ALGO_EXT 
#endif

#define BL_ALG_VERSION 110
/*
错误码
*/
enum FPALG_ERR_CODE
{
	FP_EC_OK,                      // 成功
	FP_EC_ERR_OUTOFMEMORY,         // 内存不足
	FP_EC_ERR_NULLREFERENCE,       // 空指针
	FP_EC_ERR_WRONG_PARAM,         // 输入参数不正确
	FP_EC_ERR_EMPTY_TEMPLATE,      //模板为空
	FP_EC_ERR_CREATE_IMG,          //图像创建失败
	FP_EC_ERR_CREATE_TEMPLATE,     //模板创建失败
	FP_EC_ERR_EXTRACT_TEMPLATE,    //提取模板失败
	FP_EC_ERR_UNKNOWN              // 未知错误
};

/*
算法类型
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
	algorithm_type_t nAlgorithmType;      // 算法类型选择
	unsigned int nMaxTemplateSize;        // 多模板最大字节数
	unsigned char nMaxNbrofSubtemplates;  // 多模板所含子模板最大个数
	unsigned char bSupport360Rotate;      // 是否支持360度匹配, 1表示支持，0表示不支持
}BL_INIT_PARAM, *PBL_INIT_PARAM;

/*
指纹模板结构体
*/
typedef struct BL_TEMPLATE_tag
{
	unsigned char* pTemplateData;      //模板数据
	unsigned int  templateType;       //模板类型
	unsigned int   templateSize;       //模板数据的大小
}BL_TEMPLATE, *PBL_TEMPLATE;


/*
获取库的版本号
*/	
BTL_ALGO_EXT int  bl_Alg_GetVersion(
    int* pVersion                 //[out] 返回算法库的版本号
    );

/*
初始化
*/	
BTL_ALGO_EXT int  bl_Alg_Init(
    unsigned char nAlgID,          //[in] 指定算法ID
    const BL_INIT_PARAM* pParam    //[in] 算法参数
    );
/*
释放资源
*/	
BTL_ALGO_EXT int  bl_Alg_UnInit(
    unsigned char nAlgID           //[in] 指定算法ID
    );

/*
图像质量判断
*/
BTL_ALGO_EXT int  bl_Alg_ImageQuality(
        const unsigned char* imgdata,  //[in] 指纹图像数据
		int width,                     //[in] 图像宽，BF3290为112
		int height,                    //[in] 图像高，BF3290为96
		unsigned char* pQuality,       //[out] 返回图像质量，范围[0,100]
		unsigned char* pArea,          //[out] 返回图像有效面积，范围[0,100]
		unsigned char* pCondition      //[out] 返回图像干湿程度，范围[0,100]
		);

/*
指纹录入
*/
BTL_ALGO_EXT int  bl_Alg_Enroll(
    unsigned char nAlgID,               //[in] 指定算法ID
    const unsigned char* imgdata,       //[in] 指纹图像数据
    int width,                          //[in] 图像宽，BF3290为112
    int height,                         //[in] 图像高，BF3290为96
    const PBL_TEMPLATE pblTemplate,     //[in] 提前提取的指纹模板，传入这个模板可以加速录入，如果无输入0
    int* pAcceptedNum,                  //[out] 已成功录入模板数
    unsigned char* pCoverage,           //[out] 指纹录入覆盖面积，范围[0,100]
    unsigned char* pNbrOfIslands        //[out] 孤岛数量
    );

/*
结束指纹录入，并生成模板
*/			
BTL_ALGO_EXT int  bl_Alg_FinalizeEnroll(
    unsigned char nAlgID,                   //[in] 指定算法ID
    PBL_TEMPLATE pblMultiTemplates        //[out] 返回已经生成的模板数据，支持预先分配数据内存
    );
/*
取消指纹录入
*/	
BTL_ALGO_EXT int  bl_Alg_CancelEnroll(
    unsigned char nAlgID                   //[in] 指定算法ID
    );

/*
指纹认证，使用图像
*/	
BTL_ALGO_EXT int  bl_Alg_Verify(
    unsigned char nAlgID,                   //[in] 指定算法ID
    const unsigned char* imgdata,           //[in] 指纹图像数据
	int width,                              //[in] 图像宽
	int height,                             //[in] 图像高
    const PBL_TEMPLATE* pblMultiTemplates,  //[in] 已录入模板数组 
	unsigned char nNbrOfMultiTemplates,     //[in] 已录入模板数量	
    int nFarAccepted,                       //[in] FAR
	int* pMatchIndex                        //[out] 匹配成功的模板序号，从0开始，-1表示匹配失败
	);

/*
指纹认证，使用模板
*/	
BTL_ALGO_EXT int  bl_Alg_VerifyT(
    unsigned char nAlgID,                   //[in] 指定算法ID
    const PBL_TEMPLATE pblTemplate,         //[in] 待认证指纹模板
    const PBL_TEMPLATE* pblMultiTemplates,  //[in] 已录入多模板数组 
	unsigned char nNbrOfMultiTemplates,     //[in] 已录入模板数量	
	int nFarAccepted,                       //[in] FAR
	int* pMatchIndex                        //[out] 匹配成功的模板序号，从0开始，-1表示匹配失败
	);

/*
指纹模板抽取
*/
BTL_ALGO_EXT int  bl_Alg_ExtractTemplate(
    unsigned char nAlgID,                  //[in] 指定算法ID
	unsigned char* imgdata,                //[in] 指纹图像数据
	int width,                             //[in] 图像宽
	int height,                            //[in] 图像高
	PBL_TEMPLATE pblTemplate             //[out] 提取到的模板数据，支持预先分配数据内存
	);	

/*
模板动态更新
*/
BTL_ALGO_EXT int bl_Alg_UpdateMutilTemplates(
    unsigned char nAlgID,                   //[in] 指定算法ID
    const PBL_TEMPLATE pblTemplate,         //[in] 当前指纹模板
    const PBL_TEMPLATE pblMultiTemplates,   //[in] 待更新多模板
    PBL_TEMPLATE pblUpdateMultiTemplates    //[out] 更新后多模板，支持预先分配数据内存
	);

/*
删除模板
*/
BTL_ALGO_EXT int bl_Alg_DeleteTemplate(
    PBL_TEMPLATE pblTemplate
    );

/*
图像增强
*/
BTL_ALGO_EXT int bl_Alg_Enhance(
    unsigned char* imgdata,               //[in] 指纹图像数据
    int width,                            //[in] 图像宽
    int height                            //[in] 图像高
    );

#endif


		
