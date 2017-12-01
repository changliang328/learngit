/* -------------------------------------------------------------------- */
/* ------------------kilby--------------------------------------------- */
/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
#define CHIP_ID_KILBY	(0xD0F0)
u8 bl_spi_read_reg_kilby(u8 nRegID)
{
	int ret;
    u8 nAddr;
    u8 data_tx[READREG_CMD_SIZE];

    nAddr = nRegID & 0x7F;
    nAddr |= 0x80;

    data_tx[4] = nAddr;
    data_tx[5] = 0xff;
    data_tx[6] = 0xff;
    
	data_tx[0] = READ_REG; 			 		 
	data_tx[1] = 0;					 
	data_tx[2] = 3;				 		
	data_tx[3] = 0;
	
	ret = read(g_bl_data->devfd, data_tx, READREG_CMD_SIZE);
	BTL_DEBUG(" nRegID=%x value=%x ++\n",nRegID,data_tx[1]);
    return data_tx[1];
}

/* -------------------------------------------------------------------- */
u8 bl_spi_read_frame_kilby(struct bl_fingerprint_data *bl_data,u32 len)
{
	int ret;
    u8 nAddr;
    u8 *data_tx = bl_data->tx_buf;
	BTL_DEBUG(" len=%d ++\n",len);
	//bl_spi_write_reg(bl_data->chip_params->hostcmd_reg.addr, MODE_FG_PRINT);

    nAddr = REGA_FINGER_CAP;
  	nAddr |= 0x80;

    data_tx[4] = nAddr;
    data_tx[5] = 0xff;
    
	data_tx[0] = READ_FRAME; 			 		 
	data_tx[1] = 0;					 
	data_tx[2] = len&0xff;				 		
	data_tx[3] = (len>>8)&0xff;
	
	ret = read(g_bl_data->devfd, data_tx, len);
    return ret;
}

/* -------------------------------------------------------------------- */
u8 bl_spi_write_reg_kilby(u8 nRegID, u8 value)
{
    u8 nAddr;
    u8 data_tx[REG_CMD_SIZE];
	BTL_DEBUG(" nRegID=%x,value=%x ++\n",nRegID,value);
    nAddr = nRegID & 0x7f ;
    //nAddr |= 0x80;

    data_tx[0] = nAddr;
    data_tx[1] = value;

    return write(g_bl_data->devfd, data_tx, REG_CMD_SIZE);
}

int bl_dev_init_kilby(struct bl_fingerprint_data *bl_data)
{
	int i = 0;
	struct bl_chip_params *chip_params = bl_data->chip_params;
	struct bl_reg_value *params = chip_params->params;
	
	BTL_DEBUG("  ++");
	bl_spi_write_reg_kilby(0x7f,0xaa);
	while(params[i].addr != 0xff)
	{
		bl_spi_write_reg_kilby(params[i].addr, params[i].value);
		i++;
	}
	return 0;
}


int bl_interrupt_init_kilby(struct bl_fingerprint_data *bl_data)
{
	struct bl_chip_params *chip_params = bl_data->chip_params;
     BTL_DEBUG("  ++\n");

	bl_spi_write_reg_kilby(0x65, 0x12);
	
	return 0;
}


int bl_capture_init_kilby(struct bl_fingerprint_data *bl_data)
{
	struct bl_chip_params *chip_params = bl_data->chip_params;
	     BTL_DEBUG("  ++\n");
	bl_spi_write_reg_kilby(0x65, 0x1);
	return 0;
}

u8 bl_getIntStatus_kilby(struct bl_fingerprint_data *bl_data)
{
    u8 unStatus = 0;
    unStatus = bl_spi_read_reg_kilby(0x65);
    bl_data->nStatus = unStatus;
    BTL_DEBUG("nStatus=%2x",unStatus);
    return unStatus;
}

int bl_read_chipid_kilby(struct bl_fingerprint_data *bl_data)
{
	u32 chip_id;
	u8 regValue;
	u8 version;
	
	BTL_DEBUG("  ++\n");

	regValue = bl_spi_read_reg_kilby(0x1);
	chip_id = regValue << 8;
	regValue = bl_spi_read_reg_kilby(0x30);
	chip_id += regValue;

	BTL_DEBUG(" chip_id=0x%x,version=0x%x\n",chip_id,version);
	if(CHIP_ID_KILBY != chip_id)
		return -1;
	
	bl_data->chipid = chip_id;
	return chip_id;
}

/*******************************************************
init params for kilby
*******************************************************/
static struct bl_reg_value reg_values_kilby[] = {
{0x04,0xf8},
{0x03,0xe3},	
{0x05,0xf9},	
{0x06,0xfb},	
{0x08,0xaa},
{0x33,0xf8},
{0x32,0xff},
{0x34,0xfb},
{0x35,0xf4},
{0x37,0xf8},	
{0x44,0x00},	
{0x45,0x00},	
{0x38,0x46},	
{0x39,0x16},	
{0x08,0x00},	
{0x62,0x02},	
{0x16,0x3b},
{0x20,0xff}, //fd_threshold_l
{0x21,0x10}, //fd_threshold_h
{0x40,0x00}, //dac1
{0x41,0xff}, //dac2
{0x3c,0x28}, //sensor_mode
{0x3e,0x80}, //pga1?pga2
{0x3f,0x00}, //cds
{0xff,0xff},
};
static struct bl_chip_params chip_params_kilby = {
	.dev_init = bl_dev_init_kilby,
	.interrupt_init = bl_interrupt_init_kilby,
	.capture_init = bl_capture_init_kilby,
	.get_int_status = bl_getIntStatus_kilby,	
	.read_chipid = bl_read_chipid_kilby,
	.chipid = 0xd0f0,
	.params = reg_values_kilby,
	.hostcmd_reg = {0x34,0x0},
	.fddacp_reg = {0x1d,0x40},
	.capdacp_reg = {0x1e,0x40},
	.fdgain_reg = {0x31,0x38},
	.capgain_reg = {0x31,0x38},
	.width = 112,
	.height = 88,
};



