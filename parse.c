
// C program to read particular bytes
// from the existing file
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Maximum range of bytes
#define MAX 1000
typedef struct{


    // GGA - Global Positioning System Fixed Data
    float nmea_longitude;
    float nmea_latitude;
    float utc_time;
    float speed_k;
    // RMC - Recommended Minimmum Specific GNS Data
    int date;
    char ns, ew;

} GPS_t;
typedef struct {
		float co2_ppm;
		float temperature;
		float relative_humidity;
	} CO_t;
struct sps30_measurement {
    float mc_1p0;
    float mc_2p5;
    float mc_4p0;
    float mc_10p0;
    float nc_0p5;
    float nc_1p0;
    float nc_2p5;
    float nc_4p0;
    float nc_10p0;
    float typical_particle_size;
};
typedef struct DATA
{
	uint32_t SO_ppm;
	GPS_t GPS_Data;
	CO_t CO_Data;
	struct sps30_measurement PM_Data;
} CollatedData;

// Filename given as the command
// line argument
int main(int argc, char* argv[])
{
	CollatedData collatedData;
	// Pointer to the file to be
	// read from
	FILE* fileptr;
	long filelen;
	char* buffer;

	// Stores the bytes to read
	char str[MAX];

	// If the file exists and has
	// read permission
	fileptr = fopen(argv[1], "rb");

	if (fileptr == NULL) {
		return 1;
	}

	fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
	filelen = ftell(fileptr);             // Get the current byte offset in the file
	rewind(fileptr);                      // Jump back to the beginning of the file

	buffer = (char *)malloc(filelen * sizeof(char)); // Enough memory for the file
	fread(buffer, filelen, 1, fileptr); // Read in the entire file
	
	
	fclose(fileptr); // Close the file

	memcpy(&collatedData, buffer, sizeof(collatedData));

	printf("SO2 SENSOR COLLECTION:\n\t%d ppm SO2\n", collatedData.SO_ppm);
	printf("CO2 SENSOR COLLECTION:\n\t%f ppm CO2\n\t%f °C\n\t%f %%RH\n", collatedData.CO_Data.co2_ppm, collatedData.CO_Data.temperature, collatedData.CO_Data.relative_humidity);
	printf("PM SENSOR COLLECTION:\n"
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
			collatedData.PM_Data.mc_1p0, collatedData.PM_Data.mc_2p5, collatedData.PM_Data.mc_4p0, collatedData.PM_Data.mc_10p0, collatedData.PM_Data.nc_0p5, collatedData.PM_Data.nc_1p0,
			collatedData.PM_Data.nc_2p5, collatedData.PM_Data.nc_4p0, collatedData.PM_Data.nc_10p0, collatedData.PM_Data.typical_particle_size);
	return 0;
}
