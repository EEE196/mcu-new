#include "test.h"


void init_pm( void )
{
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
	printf("PM INIT SUCCESSFUL\n");
	ret = sps30_start_measurement();
	if (ret < 0)
		printf("error starting measurement\n");

}

void init_sd( void )
{
	do{
		FATFS       FatFs;                //FatFs handle
		FRESULT     fres;                 //Result after operations
		char        buf[100];

		//Mount the SD Card
		fres = f_mount(&FatFs, "", 1);    //1=mount now
		if (fres != FR_OK)
		{
			printf("No SD Card found : (%i)\r\n", fres);
		}
		else {
			printf("SD INIT SUCCESSFUL\n");
		}

	}
	while(false);
}

void init_gps( void )
{
	printf("GPS INIT SUCCESSFUL\n");
	GPS_Init();
}


void init_lora( void )
{

	if (!rfm95_init(&rfm95_handle)) {
		printf("RFM95 init failed\n\r");
	}
	else
	{
		printf("RFM95 INIT SUCCESSFUL\n");
	}

	return rfm95_handle;
}

void init_co( void )
{
	uint16_t interval_in_seconds = 2;

	while (scd30_probe() != NO_ERROR) {
		printf("SCD30 sensor probing failed\n");
		sensirion_sleep_usec(1000000u);
	}
	printf("CO2 INIT SUCCESSFUL\n");

	scd30_set_measurement_interval(interval_in_seconds);
	sensirion_sleep_usec(20000u);
	scd30_start_periodic_measurement(0);
}
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

void test_co(void) {
	float co2_ppm, temperature, relative_humidity;
	int16_t err;
	uint16_t interval_in_seconds = 2;

	/* Initialize I2C */
	sensirion_i2c_init();

	/* Busy loop for initialization, because the main loop does not work without
	 * a sensor.
	 */
	while (scd30_probe() != NO_ERROR) {
		printf("SCD30 sensor probing failed\n");
		sensirion_sleep_usec(1000000u);
	}
	printf("SCD30 sensor probing successful\n");

	scd30_set_measurement_interval(interval_in_seconds);
	sensirion_sleep_usec(20000u);
	scd30_start_periodic_measurement(0);

	while (1) {
		uint16_t data_ready = 0;
		uint16_t timeout = 0;

		/* Poll data_ready flag until data is available. Allow 20% more than
		 * the measurement interval to account for clock imprecision of the
		 * sensor.
		 */
		for (timeout = 0; (100000 * timeout) < (interval_in_seconds * 1200000);
				++timeout) {
			err = scd30_get_data_ready(&data_ready);
			if (err != NO_ERROR) {
				printf("Error reading data_ready flag: %i\n", err);
			}
			if (data_ready) {
				break;
			}
			sensirion_sleep_usec(100000);
		}
		if (!data_ready) {
			printf("Timeout waiting for data_ready flag\n");
			continue;
		}

		/* Measure co2, temperature and relative humidity and store into
		 * variables.
		 */
		err =
				scd30_read_measurement(&co2_ppm, &temperature, &relative_humidity);
		if (err != NO_ERROR) {
			printf("error reading measurement\n");

		} else {
			printf("measured co2 concentration: %0.2f ppm, "
					"measured temperature: %0.2f degreeCelsius, "
					"measured humidity: %0.2f %%RH\n",
					co2_ppm, temperature, relative_humidity);
		}
	}

	scd30_stop_periodic_measurement();
}

void init_all(void)
{
	rfm95_handle_t loraHandle;
	FIL file;

	init_pm();
	init_sd();
	init_lora();
	init_co();
	init_gps();



}
// Create the handle for the RFM95 module.
