#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <usart.h>
#include <spi.h>
#include <tim.h>

#include "fatfs.h"
#include "../pm2.5/sps30.h"
#include "../gps/gps.h"
#include "../lora/rfm95.h"


void test_sd( void )
{
	FATFS       FatFs;                //FatFs handle
	FIL         fil;                  //File handle
	FRESULT     fres;                 //Result after operations
	char        buf[100];

	do
	{
		//Mount the SD Card
		fres = f_mount(&FatFs, "", 1);    //1=mount now
		if (fres != FR_OK)
		{
			printf("No SD Card found : (%i)\r\n", fres);
			break;
		}
		printf("SD Card Mounted Successfully!!!\r\n");

		//Read the SD Card Total size and Free Size
		FATFS *pfs;
		DWORD fre_clust;
		uint32_t totalSpace, freeSpace;

		f_getfree("", &fre_clust, &pfs);
		totalSpace = (uint32_t)((pfs->n_fatent - 2) * pfs->csize * 0.5);
		freeSpace = (uint32_t)(fre_clust * pfs->csize * 0.5);

		printf("TotalSpace : %lu bytes, FreeSpace = %lu bytes\n", totalSpace, freeSpace);

		//Open the file
		fres = f_open(&fil, "EmbeTronicX.txt", FA_WRITE | FA_READ | FA_CREATE_ALWAYS);
		if(fres != FR_OK)
		{
			printf("File creation/open Error : (%i)\r\n", fres);
			break;
		}

		printf("Writing data!!!\r\n");
		//write the data
		f_puts("Mark Guiang", &fil);

		//close your file
		f_close(&fil);

		//Open the file
		fres = f_open(&fil, "EmbeTronicX.txt", FA_READ);
		if(fres != FR_OK)
		{
			printf("File opening Error : (%i)\r\n", fres);
			break;
		}

		//read the data
		f_gets(buf, sizeof(buf), &fil);

		printf("Read Data : %s\n", buf);

		//close your file
		f_close(&fil);
		printf("Closing File!!!\r\n");
#if 0
		//Delete the file.
		fres = f_unlink(EmbeTronicX.txt);
		if (fres != FR_OK)
		{
			printf("Cannot able to delete the file\n");
		}
#endif
	} while( false );

	//We're done, so de-mount the drive
	f_mount(NULL, "", 0);
	printf("SD Card Unmounted Successfully!!!\r\n");
}

void test_pm(void) {
	struct sps30_measurement m;
	int16_t ret;

	/* Initialize I2C bus */
	/* Busy loop for initialization, because the main loop does not work without
	 * a sensor.
	 */
	while (sps30_probe() != 0) {
		printf("SPS sensor probing failed\n");
		sensirion_sleep_usec(1000000); /* wait 1s */
	}
	printf("SPS sensor probing successful\n");

	uint8_t fw_major;
	uint8_t fw_minor;
	ret = sps30_read_firmware_version(&fw_major, &fw_minor);
	if (ret) {
		printf("error reading firmware version\n");
	} else {
		printf("FW: %u.%u\n", fw_major, fw_minor);
	}

	char serial_number[SPS30_MAX_SERIAL_LEN];
	ret = sps30_get_serial(serial_number);
	if (ret) {
		printf("error reading serial number\n");
	} else {
		printf("Serial Number: %s\n", serial_number);
	}

	ret = sps30_start_measurement();
	if (ret < 0)
		printf("error starting measurement\n");
	printf("measurements started\n");

	while (1) {
		sensirion_sleep_usec(SPS30_MEASUREMENT_DURATION_USEC); /* wait 1s */
		ret = sps30_read_measurement(&m);
		if (ret < 0) {
			printf("error reading measurement\n");

		} else {
			printf("measured values:\n"
					"\t%0.2f pm1.0\n"
					"\t%0.2f pm2.5\n"
					"\t%0.2f pm4.0\n"
					"\t%0.2f pm10.0\n"
					"\t%0.2f nc0.5\n"
					"\t%0.2f nc1.0\n"
					"\t%0.2f nc2.5\n"
					"\t%0.2f nc4.5\n"
					"\t%0.2f nc10.0\n"
					"\t%0.2f typical particle size\n\n",
					m.mc_1p0, m.mc_2p5, m.mc_4p0, m.mc_10p0, m.nc_0p5, m.nc_1p0,
					m.nc_2p5, m.nc_4p0, m.nc_10p0, m.typical_particle_size);
		}
	}
}
void test_gps(void) {
	GPS_Init();
}

rfm95_handle_t rfm95_handle = {
		.spi_handle = &hspi2,
		.nss_port = RFM95_NSS_GPIO_Port,
		.nss_pin = RFM95_NSS_Pin,
		.nrst_port = RFM95_NRST_GPIO_Port,
		.nrst_pin = RFM95_NRST_Pin,
		.irq_port = RFM95_DIO0_GPIO_Port,
		.irq_pin = RFM95_DIO0_Pin,
		.dio5_port = RFM95_DIO5_GPIO_Port,
		.dio5_pin = RFM95_DIO5_Pin,
		.device_address = {0xDE, 0xAD, 0xBE, 0xEF},
		.application_session_key = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		.network_session_key = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		.reload_frame_counter = NULL,
		.save_frame_counter = NULL
};
void test_lora(void) {



	if (!rfm95_init(&rfm95_handle)) {
		printf("RFM95 init failed\n\r");
	} else
	{
		uint8_t data_packet[] = {
				0x69, 0x45, 0x45, 0x45
		};
		if (!rfm95_send_data(&rfm95_handle, data_packet, sizeof(data_packet))) {
			printf("RFM95 send failed\n\r");
		}
		else{
			printf("OWSHI\n");
		}
	}
}




// Create the handle for the RFM95 module.
