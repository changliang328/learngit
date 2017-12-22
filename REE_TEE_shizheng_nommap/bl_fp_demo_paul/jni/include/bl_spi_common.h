#ifndef _BL_SPI_COMMON_H_
#define _BL_SPI_COMMON_H_
struct bl_fingerprint_data;
u8 bl_spi_read_reg(u8 nRegID);
u8 bl_spi_write_reg(u8 nRegID, u8 value);
u8 bl_spi_write_reg_bit(u8 nRegID, u8 bit, u8 value);
int bl_read_chipid(struct bl_fingerprint_data *bl_data);
int bl_dev_init(struct bl_fingerprint_data *bl_data);
int bl_interrupt_init(struct bl_fingerprint_data *bl_data);
int bl_capture_init(struct bl_fingerprint_data *bl_data);
int bl_capture_init_framenum(struct bl_fingerprint_data *bl_data,int framenum);
u8 bl_getIntStatus(struct bl_fingerprint_data *bl_data);
u8 bl_spi_read_frame(struct bl_fingerprint_data *bl_data,u32 len);
int bl_set_frame_size(struct bl_fingerprint_data *bl_data,u32 height,u32 width);
int bl_set_frame_num(struct bl_fingerprint_data *bl_data,u32 frame_num);
int bl_set_gain_dacp(struct bl_fingerprint_data *bl_data,u8 mode,u8 gain,u8 dacp);
int esd_recovery_chip_params(struct bl_fingerprint_data *bl_data);
void bl_hexdump(const unsigned char *buf, const int num);
int bl_read_chipid_3290(struct bl_fingerprint_data *bl_data);
int bl_read_chipid_typeA(struct bl_fingerprint_data *bl_data);
int destroy_fingerprint_data(struct bl_fingerprint_data *bl_data);
int init_new_fingerprint_data(struct bl_fingerprint_data *bl_data,int fd);
int bl_set_chip_idlemode(struct bl_fingerprint_data *bl_data);

struct bl_fingerprint_data *bl_fingerprint_data_new(int fd, void *config);
#define CHIP_ID_LOW		                (0x83)
#define CHIP_ID_HIGH	                (0x51)

#define RESET_PIN_FAILED	(1)
#define SPI_PIN_FAILED		(2)
#define INT_PIN_FAILED		(3)

enum bl_spi_cmd {
	ERROR_CMD		= 0x00,
	READ_REG		= 0x01,
	WRITE_REG		= 0x02,
	READ_FRAME		= 0x03,
};
#define BL_READCMD_HEADER_SIZE	(4)
#define REG_CMD_SIZE	(2)
#define READREG_CMD_SIZE	(BL_READCMD_HEADER_SIZE + REG_CMD_SIZE)

#endif
