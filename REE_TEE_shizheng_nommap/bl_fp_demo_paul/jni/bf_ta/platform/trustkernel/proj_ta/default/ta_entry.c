/*
 * Copyright (c) 2014, STMicroelectronics International N.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdint.h>
#include <tee_api.h>
#include <tee_internal_api_extensions.h>
#include <tee_ta_api.h>
#include "bf_log.h"
#include "bf_core.h"

#define TEE_NORMAL_CMD 9527
#define TEEC_MEMREF_TEMP_INOUT      0x00000007
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

	TEE_UnmaskCancellation();
	return TEE_SUCCESS;
}

/* Called each time a session is closed */
void TA_CloseSessionEntryPoint(void *pSessionContext)
{
	(void)pSessionContext;
	BF_LOG("TA_CloseSessionEntryPoint");
}


/* Called when a command is invoked */
TEE_Result  TA_InvokeCommandEntryPoint(void *pSessionContext,
		                                          uint32_t nCommandID, 
		                                          uint32_t nParamTypes,
		                                          TEE_Param pParams[4])
{
	(void) pSessionContext;
	uint32_t *buff;
	uint32_t buff_size = 0;
	EMSG("fp TA_InvokeCommandEntryPoint, %d", nCommandID);

	BF_LOG("TA_InvokeCommandEntryPoint, %d", nCommandID);	
	switch (nCommandID)
	{
	case TEE_NORMAL_CMD:
		if (nParamTypes != TEE_PARAM_TYPES(     	
					TEEC_MEMREF_TEMP_INOUT,  //TEE_PARAM_TYPE_MEMREF_INOUT, 
					TEE_PARAM_TYPE_NONE, 
					TEE_PARAM_TYPE_NONE, 
					TEE_PARAM_TYPE_NONE)) 
		{
			BF_LOG("+%d %s Wrong param types. \n",__LINE__,__func__);
			return TEE_ERROR_BAD_PARAMETERS;
		}
	buff  = (uint32_t*)(pParams[0].memref.buffer);
	buff_size = pParams[0].memref.size;
	bf_core_handler(buff,&buff_size);
	//bf_core_handler((char*)(pParams[0].memref.buffer),(unsigned int*)(&pParams[0].memref.size));
	break;
			
	default:
		BF_LOG("+%d %s Wrong nCommandID. \n",__LINE__,__func__);
		return TEE_ERROR_NOT_IMPLEMENTED;
	}
	
	BF_LOG("\n");
	
	return TEE_SUCCESS;
}

