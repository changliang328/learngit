#ifndef __BF_MMI_H__
#define __BF_MMI_H__

/*
[31-24]:[23-16]:[15-8]:[7-0]
 CMD	|ARG1	|ARG2 |RESULT
*/
//#define BITS_PER_LONG 64
//#define GENMASK(h, l) \
	(((~0UL) << (l)) & (~0UL >> (BITS_PER_LONG - 1 - (h))))

typedef  enum
{
    MMI_TEST_NONE = 0,
    MMI_TYPE_AUTO_TEST,
    MMI_TYPE_INTERRUPT_TEST ,
    MMI_TYPE_AUTO_GET_BEST_IMAGE,
    MMI_TYPE_GET_RAWIMAGE_WITHOUT_INT,
//5
    MMI_TYPE_GET_RAWIMAGE_WITH_INT ,
	MMI_TYPE_GET_HEIGHT_WIDTH ,
    MMI_TYPE_SNR_WHITE_IMAGE_TEST ,
} mmi_test_type;

typedef enum    //for struct mmi_info. testResult
{
	BF_MMI_TEST_OK = 0x60,
	BF_MMI_TEST_FAILED,
    BF_MMI_TEST_NONE,
    BF_MMI_TESTING,
	BF_MMI_CANCEL,
} mmi_test_result;

#define BIT_OFFSET_OF_CMD (24)
#define BIT_OFFSET_OF_ARG1 (16)
#define BIT_OFFSET_OF_ARG2 (8)
#define BIT_OFFSET_OF_RESULT (0)

#define BITS_MASK_OF_CMD (0xff << BIT_OFFSET_OF_CMD)
#define BITS_MASK_OF_ARG1 (0xff << BIT_OFFSET_OF_ARG1)
#define BITS_MASK_OF_ARG2 (0xff << BIT_OFFSET_OF_ARG2)
#define BITS_MASK_OF_RESULT (0xff << BIT_OFFSET_OF_RESULT)
	
#define set_bits(p, v, b, m)	(((p) & ~(m)) | ((v) << (b)))
#define get_bits(p, b, m)	(((p)&(m)) >> (b))

static inline uint32_t set_workstate_cmd(uint32_t workstate,uint32_t value)
{
	return set_bits(workstate, value, BIT_OFFSET_OF_CMD, BITS_MASK_OF_CMD);
}

static inline uint32_t set_workstate_result(uint32_t workstate,uint32_t value)
{
	return set_bits(workstate, value, BIT_OFFSET_OF_RESULT, BITS_MASK_OF_RESULT);
}

static inline uint32_t set_workstate_arg1(uint32_t workstate,uint32_t value)
{
	return set_bits(workstate, value, BIT_OFFSET_OF_ARG1, BITS_MASK_OF_ARG1);
}

static inline uint32_t set_workstate_arg2(uint32_t workstate,uint32_t value)
{
	return set_bits(workstate, value, BIT_OFFSET_OF_ARG2, BITS_MASK_OF_ARG2);
}

static inline uint32_t get_workstate_cmd(uint32_t workstate)
{
	return get_bits(workstate, BIT_OFFSET_OF_CMD, BITS_MASK_OF_CMD);
}

static inline uint32_t get_workstate_result(uint32_t workstate)
{
	return get_bits(workstate, BIT_OFFSET_OF_RESULT, BITS_MASK_OF_RESULT);
}

static inline uint32_t get_workstate_arg1(uint32_t workstate)
{
	return get_bits(workstate, BIT_OFFSET_OF_ARG1, BITS_MASK_OF_ARG1);
}

static inline uint32_t get_workstate_arg2(uint32_t workstate)
{
	return get_bits(workstate, BIT_OFFSET_OF_ARG2, BITS_MASK_OF_ARG2);
}


#endif
