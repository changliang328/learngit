#ifndef BTL_FPDATA_H
#define BTL_FPDATA_H

typedef struct fp_data
{
	uint32_t cmd;
	uint32_t fid;
	uint32_t matched_fid;
	char template_path[128];
	uint32_t pathlen;
	uint32_t remaining_times;
	uint32_t fingernum;
	int32_t mode;
	uint16_t nav_data;
	uint32_t sensor_id;
	hw_auth_token_t hat;
	uint32_t hat_len;
	uint64_t challenge;
	uint64_t auth_id;
	int32_t status;
	uint32_t gid;
}fp_data_t;


typedef struct fp_img_data
{
	uint32_t cmd;
	unsigned char imagebuf[96*96];
}fp_img_data_t;


#endif
