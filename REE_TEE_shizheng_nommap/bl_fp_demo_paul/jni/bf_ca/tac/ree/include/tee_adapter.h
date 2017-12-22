#ifndef __TEE_ADAPTER_H__
#define __TEE_ADAPTER_H__

#ifdef __cplusplus
extern "C" {
#endif


int tee_adapter_init_tzapp(void);
void tee_adapter_destroy_tzapp(void);
int tee_adapter_invoke_command(char* buf, unsigned int len);
int ts_ca_get_key(void *key_data, uint32_t *key_len);


#ifdef __cplusplus
}
#endif

#endif

