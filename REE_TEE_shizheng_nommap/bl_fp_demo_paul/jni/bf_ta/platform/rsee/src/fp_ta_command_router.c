#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include <tee_api.h>
#include <tee_internal_api_extensions.h>
#include <tee_internal_api.h>
#include <tee_ta_api.h>
#include <rsee_spi.h>
#include "bf_core.h"
#include "bf_log.h"
#include "tee_adapter.h"

#define TEE_NORMAL_CMD 9527

/*
 * Trusted Application Entry Points
 */

/* Called each time a new instance is created */
TEE_Result TA_CreateEntryPoint(void)
{
	BF_LOG("TA_CreateEntryPoint");
	return TEE_SUCCESS;
}

/* Called each time an instance is destroyed */
void TA_DestroyEntryPoint(void)
{
	BF_LOG("TA_DestroyEntryPoint");
}

/* Called each time a session is opened */
TEE_Result TA_OpenSessionEntryPoint(uint32_t nParamTypes,
				    TEE_Param pParams[4],
				    void **ppSessionContext)
{
	(void)nParamTypes;
	(void)pParams;
	(void)ppSessionContext;
	BF_LOG("TA_OpenSessionEntryPoint");
	return TEE_SUCCESS;
}

/* Called each time a session is closed */
void TA_CloseSessionEntryPoint(void *pSessionContext)
{
	BF_LOG("TA_CloseSessionEntryPoint");
	(void)pSessionContext;
}


/* Called when a command is invoked */
TEE_Result TA_InvokeCommandEntryPoint(void *pSessionContext,
		                                          uint32_t nCommandID, 
		                                          uint32_t nParamTypes,
		                                          TEE_Param pParams[4])
{
	(void) pSessionContext;
	BF_LOG("(+%d)%s CommandID = %u\n", __LINE__, __func__, nCommandID);
	switch (nCommandID)
	{
		case TEE_NORMAL_CMD:
			if (nParamTypes != TEE_PARAM_TYPES(     	
						TEE_PARAM_TYPE_MEMREF_INOUT, 
						TEE_PARAM_TYPE_NONE, 
						TEE_PARAM_TYPE_NONE, 
						TEE_PARAM_TYPE_NONE)) 
			{
				BF_LOG("+%d %s Wrong param types. \n",__LINE__,__func__);
				return TEE_ERROR_BAD_PARAMETERS;
			}
			bf_core_handler((char*)(pParams[0].memref.buffer),(unsigned int)(pParams[0].memref.size));
			break;
			
		default:
			BF_LOG("+%d %s Wrong nCommandID. \n",__LINE__,__func__);
			return TEE_ERROR_NOT_IMPLEMENTED;
	}
	
	BF_LOG("\n");
	
	return TEE_SUCCESS;
}




