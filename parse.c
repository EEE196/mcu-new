
// C program to read particular bytes
// from the existing file
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Maximum range of bytes
#define MAX 30000
typedef struct{


    // GGA - Global Positioning System Fixed Data
    float nmea_longitude;
    float nmea_latitude;
    float utc_time;
    // RMC - Recommended Minimmum Specific GNS Data
    int date;
} GPS_t;
typedef struct {
		float co2_ppm;
		float temperature;
		float relative_humidity;
	} CO_t;
struct sps30_measurement {
    float mc_2p5;
    float mc_10p0;
    float nc_2p5;
    float nc_10p0;
    uint16_t SO_ppm;
};
typedef struct DATA
{
	struct sps30_measurement PM_Data;
	GPS_t GPS_Data;
	CO_t CO_Data;
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

	int numChunks = bufferSize / sizeof(collatedData);
	for(int i = 0; i<numChunks; i++) {
		char* chunk = buffer + (i+sizeof(collatedData));
		memcpy(&collatedData, chunk, sizeof(collatedData));
	}

	return 0;
}
