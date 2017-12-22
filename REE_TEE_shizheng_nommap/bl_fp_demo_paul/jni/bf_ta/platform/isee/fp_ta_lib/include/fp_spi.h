#ifndef __FP_SPI_H__
#define __FP_SPI_H__

#ifdef __cplusplus
extern "C" {
#endif

enum spi_cpol {
	SPI_CPOL_0,
	SPI_CPOL_1
};

enum spi_cpha {
	SPI_CPHA_0,
	SPI_CPHA_1
};

enum spi_mlsb {
	SPI_LSB,
	SPI_MSB
};

enum spi_endian {
	SPI_LENDIAN,
	SPI_BENDIAN
};

enum spi_transfer_mode {
	FIFO_TRANSFER,
	DMA_TRANSFER,
	OTHER1,
	OTHER2,
};

enum spi_pause_mode {
	PAUSE_MODE_DISABLE,
	PAUSE_MODE_ENABLE
};
enum spi_finish_intr {
	FINISH_INTR_DIS,
	FINISH_INTR_EN,
};

enum spi_deassert_mode {
	DEASSERT_DISABLE,
	DEASSERT_ENABLE
};

enum spi_ulthigh {
	ULTRA_HIGH_DISABLE,
	ULTRA_HIGH_ENABLE
};

enum spi_tckdly {
	TICK_DLY0,
	TICK_DLY1,
	TICK_DLY2,
	TICK_DLY3
};

enum spi_irq_flag {
	IRQ_IDLE,
	IRQ_BUSY
};

enum spi_sample_sel {
	aaaaa,
	aaaaab

};
enum spi_cs_pol {
	caaaaa,
	caaaaab
};

struct mt_chip_conf {
	unsigned int setuptime;
	unsigned int holdtime;
	unsigned int high_time;
	unsigned int low_time;
	unsigned int cs_idletime;
	unsigned int ulthgh_thrsh;
	
	enum spi_sample_sel sample_sel;
	enum spi_cs_pol cs_pol;

	enum spi_cpol cpol;
	enum spi_cpha cpha;
	enum spi_mlsb tx_mlsb;
	enum spi_mlsb rx_mlsb;
	enum spi_endian tx_endian;
	enum spi_endian rx_endian;
	enum spi_transfer_mode com_mod;
	enum spi_pause_mode pause;
	enum spi_finish_intr finish_intr;
	enum spi_deassert_mode deassert;
	enum spi_ulthigh ulthigh;
	enum spi_tckdly tckdly;
};

#ifdef __cplusplus
}
#endif

#endif // __FPC_SPI_H__
