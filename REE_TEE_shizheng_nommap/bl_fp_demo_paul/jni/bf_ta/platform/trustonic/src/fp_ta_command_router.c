#include <stdint.h>
#include <taStd.h>
#include <tee_internal_api.h>
#include "bf_core.h"
#include "bf_log.h"

#define TEE_NORMAL_CMD 9527


DECLARE_TRUSTED_APPLICATION_MAIN_STACK(80000); 

extern int fp_invoke_command(char* data ,unsigned int length);


/*
 * Trusted Application Entry Points
 */

/* Called each time a new instance is created */
TEE_Result TA_EXPORT TA_CreateEntryPoint(void)
{
	TEE_Result res = 0;
	TEE_DbgPrintLnf("(+%d)%s create a new instance\n", __LINE__, __func__);
	return res;
}

/* Called each time an instance is destroyed */
void TA_EXPORT TA_DestroyEntryPoint(void)
{
	TEE_DbgPrintLnf("(+%d)%s destroy a new instance\n", __LINE__, __func__);
}

/* Called each time a session is opened */
TEE_Result TA_EXPORT TA_OpenSessionEntryPoint(uint32_t nParamTypes,
		TEE_Param pParams[4],
		void **ppSessionContext)
{
	(void)nParamTypes;
	(void)pParams;
	(void)ppSessionContext;
	TEE_DbgPrintLnf("(+%d)%s open a new session\n", __LINE__, __func__);
	return TEE_SUCCESS;
}

/* Called each time a session is closed */
void TA_EXPORT TA_CloseSessionEntryPoint(void *pSessionContext)
{
	TEE_DbgPrintLnf("(+%d)%s close an opened session\n", __LINE__, __func__);
	(void)pSessionContext;
}

/* Called when a command is invoked */
TEE_Result TA_EXPORT TA_InvokeCommandEntryPoint(void *pSessionContext,
		                                          uint32_t nCommandID, 
		                                          uint32_t nParamTypes,
		                                          TEE_Param pParams[4])
{
	(void) pSessionContext;

	switch (nCommandID)
	{
	case TEE_NORMAL_CMD:
		if (nParamTypes != TEE_PARAM_TYPES(     	
					TEE_PARAM_TYPE_MEMREF_INOUT, 
					TEE_PARAM_TYPE_NONE, 
					TEE_PARAM_TYPE_NONE, 
					TEE_PARAM_TYPE_NONE)) 
		{
			TEE_DbgPrintLnf("+%d %s Wrong param types. \n",__LINE__,__func__);
			return TEE_ERROR_BAD_PARAMETERS;
		}
			
		bf_core_handler((char*)(pParams[0].memref.buffer),(unsigned int)(pParams[0].memref.size));
		break;
			
	default:
		TEE_DbgPrintLnf("+%d %s Wrong nCommandID. \n",__LINE__,__func__);
		return TEE_ERROR_NOT_IMPLEMENTED;
	}
	
	TEE_DbgPrintLnf("\n");
	
	return TEE_SUCCESS;
}
