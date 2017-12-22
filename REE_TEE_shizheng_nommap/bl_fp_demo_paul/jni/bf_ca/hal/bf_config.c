#include "bf_types.h"
#include "cJSON.h"
#include "bf_config.h"
#include <errno.h>
#include <fcntl.h>
#include "bf_log.h"
#define DEF_STR "null"

#define countof(array) (sizeof(array)/sizeof(array[0]))

static void stats_read_json_file(const char *name, char *info, int length)
{
    int fd = -1, nread = 0;
    memset(info, 0, length); // set to default
    fd = open(name, O_RDONLY);
    if (fd < 0)
    {
        LOGE("%s: open failed, errno = %d\n", __func__, errno);
        return;
    }
    nread = read(fd, info, length);
    if ((nread <= 0) || (nread > length))
    {
        LOGE("%s: read failed, errno = %d\n", __func__, errno);
        close(fd);
        return;
    }
    info[length - 1] = '\0'; // set last string
    close(fd);
}

static void get_int_val_from_json(cJSON *root, char *name, int *val)
{
    long iTmp = 0;
    cJSON *pSub = NULL;
    pSub = cJSON_GetObjectItem(root, name);
    if(NULL == pSub)
    {
        LOGE("%s:null point set as default",__func__);
        *val = 0;
        return;
    }

    if (cJSON_Number == pSub->type)
    {
        *val = pSub->valueint;
        return;
    }
    else if (cJSON_String == pSub->type)
    {
        LOGE("%s:json type erro1, expect %d, but get %d", __func__, cJSON_Number, pSub->type);
        iTmp = atol(pSub->valuestring);
        if (iTmp >= 0)
        {
            *val = iTmp;
        }
        else
        {
            *val = 0;
        }
    }
    else
    {
        LOGE("%s:json type erro2, expect %d, but get %d", __func__, cJSON_Number, pSub->type);
        *val = 0;
    }
}

static void get_string_val_from_json(cJSON *root, char *name, char **val)
{
    cJSON *pSub = NULL;
    pSub = cJSON_GetObjectItem(root, name);
    if(NULL == pSub)
    {
        LOGE("%s:null point set as default 0.",__func__);
        *val = DEF_STR;
        return;
    }

    if (cJSON_String == pSub->type)
    {
        *val = pSub->valuestring;
    }
    else
    {
        LOGE("%s:json type erro, expect %d, but get %d", __func__, cJSON_String, pSub->type);
        *val = DEF_STR;
    }
}


int load_algo_params(cJSON * root, load_config_t *config)
{
	cJSON * item = NULL;
	
	item = cJSON_GetObjectItem(root, "nAlgorithmType");
	config->nAlgorithmType= (uint32_t)strtoul(item->valuestring, 0, 10);

	item = cJSON_GetObjectItem(root, "nMaxTemplateSize");
	config->nMaxTemplateSize= (uint32_t)strtoul(item->valuestring, 0, 10);
	
	item = cJSON_GetObjectItem(root, "nMaxNbrofSubtemplates");	
	config->nMaxNbrofSubtemplates= (uint32_t)strtoul(item->valuestring, 0, 10);

	item = cJSON_GetObjectItem(root, "bSupport360Rotate");
	config->bSupport360Rotate= (uint32_t)strtoul(item->valuestring, 0, 10);
	/*config->width= (uint32_t)strtoul(item->valuestring, 0, 10);
	config->height= (uint32_t)strtoul(item->valuestring, 0, 10);*/
	item = cJSON_GetObjectItem(root, "pb_area_max");
	config->pb_area_max= (uint32_t)strtoul(item->valuestring, 0, 10);

	item = cJSON_GetObjectItem(root, "pb_area_match_threhold");
	config->pb_area_match_threhold= (uint32_t)strtoul(item->valuestring, 0, 10);

	item = cJSON_GetObjectItem(root, "pb_area_enroll_threhold");
	config->pb_area_enroll_threhold= (uint32_t)strtoul(item->valuestring, 0, 10);

	item = cJSON_GetObjectItem(root, "far_match");
	config->far_match= (uint32_t)strtoul(item->valuestring, 0, 10);

	item = cJSON_GetObjectItem(root, "far_update");
	config->far_update= (uint32_t)strtoul(item->valuestring, 0, 10);

	item = cJSON_GetObjectItem(root, "qscore_match");
	config->qscore_match= (uint32_t)strtoul(item->valuestring, 0, 10);

	item = cJSON_GetObjectItem(root, "qscore_enroll");
	config->qscore_enroll= (uint32_t)strtoul(item->valuestring, 0, 10);

	item = cJSON_GetObjectItem(root, "dacp_direction");
	config->dacp_direction= (uint32_t)strtoul(item->valuestring, 0, 10);

	item = cJSON_GetObjectItem(root, "nXuAreaMatch");
	config->nXuAreaMatch= (uint32_t)strtoul(item->valuestring, 0, 10);

	item = cJSON_GetObjectItem(root, "nXuAreaUpdate");
	config->nXuAreaUpdate= (uint32_t)strtoul(item->valuestring, 0, 10);

	item = cJSON_GetObjectItem(root, "nXuAreaEnroll");
	config->nXuAreaEnroll= (uint32_t)strtoul(item->valuestring, 0, 10);

	item = cJSON_GetObjectItem(root, "nMaxSample");
	config->nMaxSample= (uint32_t)strtoul(item->valuestring, 0, 10);

	item = cJSON_GetObjectItem(root, "nFingerDownValue");
	config->nFingerDownValue= (uint32_t)strtoul(item->valuestring, 0, 10);

	item = cJSON_GetObjectItem(root, "nBadPixelMaxValue");
	config->nBadPixelMaxValue= (uint32_t)strtoul(item->valuestring, 0, 10);

	item = cJSON_GetObjectItem(root, "nBadPixelsNumThre");
	config->nBadPixelsNumThre= (uint32_t)strtoul(item->valuestring, 0, 10);

	BF_LOG("\n\tnAlgorithmType:%d\n\
	nMaxTemplateSize:%d\n\
	nMaxNbrofSubtemplates:%d\n\
	bSupport360Rotate:%d\n\
	width:%d\n\
	height:%d\n\
	pb_area_max:%d\n\
	pb_area_match_threhold:%d\n\
	pb_area_enroll_threhold:%d\n\
	far_match:%d\n\
	far_update:%d\n\
	qscore_match:%d\n\
	qscore_enroll:%d\n\
	dacp_direction:%d\n\
	nXuAreaMatch:%d\n\
	nXuAreaUpdate:%d\n\
	nXuAreaEnroll:%d\n\
	nMaxSample:%d\n\
	nFingerDownValue:%d\n\
	nBadPixelMaxValue:%d\n\
	nBadPixelsNumThre:%d\n",  		config->nAlgorithmType,
									config->nMaxTemplateSize,
									config->nMaxNbrofSubtemplates,
									config->bSupport360Rotate,
									config->width,
									config->height,
									config->pb_area_max,
									config->pb_area_match_threhold,
									config->pb_area_enroll_threhold,
									config->far_match,
									config->far_update,
									config->qscore_match,
									config->qscore_enroll,
									config->dacp_direction,
									config->nXuAreaMatch,
									config->nXuAreaUpdate,
									config->nXuAreaEnroll,
									config->nMaxSample,
									config->nFingerDownValue,
									config->nBadPixelMaxValue,
									config->nBadPixelsNumThre);
	return 0;
}

int load_cmdreg_params(cJSON * root, load_config_t *config)
{
	cJSON *chipparams = NULL;
	cJSON *item = NULL;
	
	chipparams = cJSON_GetObjectItem(root, "hostcmd_reg");
	item = cJSON_GetArrayItem(chipparams, 0);	
	config->hostcmd_reg.addr = (u8)strtoul(item->valuestring, 0, 16);
	item = cJSON_GetArrayItem(chipparams, 1);
	config->hostcmd_reg.value= (u8)strtoul(item->valuestring, 0, 16);
	BF_LOG("\naddr:0x%x val:0x%x",config->hostcmd_reg.addr, config->hostcmd_reg.value);	
 
	chipparams = cJSON_GetObjectItem(root, "fddacp_reg");
	item = cJSON_GetArrayItem(chipparams, 0);	
	config->fddacp_reg.addr = (u8)strtoul(item->valuestring, 0, 16);
	item = cJSON_GetArrayItem(chipparams, 1);
	config->fddacp_reg.value= (u8)strtoul(item->valuestring, 0, 16);
	BF_LOG("\naddr:0x%x val:0x%x",config->fddacp_reg.addr, config->fddacp_reg.value);	

	chipparams = cJSON_GetObjectItem(root, "capdacp_reg");
	item = cJSON_GetArrayItem(chipparams, 0);	
	config->capdacp_reg.addr = (u8)strtoul(item->valuestring, 0, 16);
	item = cJSON_GetArrayItem(chipparams, 1);
	config->capdacp_reg.value= (u8)strtoul(item->valuestring, 0, 16);
	BF_LOG("\naddr:0x%x val:0x%x",config->capdacp_reg.addr, config->capdacp_reg.value);	

	chipparams = cJSON_GetObjectItem(root, "fdgain_reg");
	item = cJSON_GetArrayItem(chipparams, 0);	
	config->fdgain_reg.addr = (u8)strtoul(item->valuestring, 0, 16);
	item = cJSON_GetArrayItem(chipparams, 1);
	config->fdgain_reg.value= (u8)strtoul(item->valuestring, 0, 16);
	BF_LOG("\naddr:0x%x val:0x%x",config->fdgain_reg.addr, config->fdgain_reg.value);	

	chipparams = cJSON_GetObjectItem(root, "capgain_reg");
	item = cJSON_GetArrayItem(chipparams, 0);	
	config->capgain_reg.addr = (u8)strtoul(item->valuestring, 0, 16);
	item = cJSON_GetArrayItem(chipparams, 1);
	config->capgain_reg.value= (u8)strtoul(item->valuestring, 0, 16);
	BF_LOG("\naddr:0x%x val:0x%x",config->capgain_reg.addr, config->capgain_reg.value);	

	return 0;
}


int load_chip_init_params(cJSON * root, load_config_t *config)
{
	int count = 0;
	int i = 0;
	cJSON * item = NULL;
	char *val = NULL;
	
	count = cJSON_GetArraySize(root);
	BF_LOG("count=%d",count);
	if(count > MAX_INIT_REGS)
	{
	  BF_LOG("the init params length exceeds the limit!\n");
	  return -1;
	}
	for(i = 0;i < count; i++)
	{
	  item = cJSON_GetArrayItem(root, i);
	  get_string_val_from_json(item, "addr", &val);
	  config->init_params[i].addr = (u8)strtoul(val, 0, 16);
	  
	  get_string_val_from_json(item, "val", &val);
	  config->init_params[i].value= (u8)strtoul(val, 0, 16);
	  BF_LOG("\naddr 0x%x value 0x%x", config->init_params[i].addr, config->init_params[i].value);
	}	
	
	return 0;
}


int load_single_chip_value(cJSON *root, load_config_t* config)
{
	char *val = NULL;

	get_string_val_from_json(root, "chipid", &val);
	config->chipid = (uint32_t) strtoul(val, 0, 16);

	get_string_val_from_json(root, "chiptype", &val);
	config->chiptype= (uint32_t) strtoul(val, 0, 10);

	get_string_val_from_json(root, "fdwidth", &val);
	config->fdwidth=(uint32_t) strtoul(val, 0, 10);
	
	get_string_val_from_json(root, "fdheight", &val);
	config->fdheight=(uint32_t) strtoul(val, 0, 10);

	get_string_val_from_json(root, "fdframe_num", &val);
	config->fdframe_num=(uint32_t) strtoul(val, 0, 10);

	get_string_val_from_json(root, "width", &val);
	config->width=(uint32_t) strtoul(val, 0, 10);

	get_string_val_from_json(root, "height", &val);
	config->height=(uint32_t) strtoul(val, 0, 10);

	get_string_val_from_json(root, "capframe_num", &val);
	config->capframe_num=(uint32_t) strtoul(val, 0, 10);

	get_string_val_from_json(root, "force_loading", &val);
	config->force_loading=(uint32_t) strtoul(val, 0, 10);
	
	BF_LOG("\n\tchipid:0x%x\n\
	chiptype: %d\n\
	fdwidth: %d\n\
	fdheight: %d\n\
	fdframe_num:%d\n\
	width: %d\n\
	height: %d\n\
	capframe_num: %d\n\
	force_loading: %d\n",config->chipid, 
						config->chiptype,
						config->fdwidth, 
						config->fdheight,
						config->fdframe_num, 
						config->width, 
						config->height, 
						config->capframe_num,
						config->force_loading);

	return 0;
}


void * bf_load_config(const char *configName)
{
    cJSON * root = NULL;
    cJSON * chipparams = NULL;
	char buf[2000];
	int ret = -1;
	
	load_config_t *config = NULL;
	config = malloc(sizeof(load_config_t));
	if (NULL ==config)
    { 
    	BF_LOG("%d malloc failed \n", errno);
    	goto err;
	}
	memset(config, 0, sizeof(load_config_t));

	/*parse cjson file*/
	stats_read_json_file(configName, buf,sizeof(buf));
	BF_LOG("configfile name:%s \n", configName);
	root = cJSON_Parse(buf);
    if (NULL == root)
    {
        BF_LOG("%d cJSON_Parse failed \n", errno);
        goto err;
    }


	/*get single value item.*/
	ret = load_single_chip_value(root, config);	
	if(ret)
		goto err;
	

	/*get cmd reg  item*/
	ret = load_cmdreg_params(root, config);
	if(ret)
		goto err;
	

	/*get algo value item*/
	chipparams = cJSON_GetObjectItem(root, "pbf_algo");
	if (NULL == chipparams)
    {
        BF_LOG("%d cJSON_Parse failed \n", errno);
        goto err;
    }
	ret = load_algo_params(chipparams, config);
	if(ret)
		goto err;


	/*get chip init array params*/
	chipparams = NULL;
	chipparams = cJSON_GetObjectItem(root, "blestech_fp");
	if (NULL ==chipparams)
    {
        BF_LOG("%d cJSON_Parse failed \n", errno);
        goto err;
    }
	ret = load_chip_init_params(chipparams, config);
	if(ret)
		goto err;  
	return (void *)config;
	
err:
	if(NULL != config)
		free(config);
	BF_LOG("load error, exit!\n");
	return NULL;
}
