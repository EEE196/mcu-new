
// C program to read particular bytes
// from the existing file
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>

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
	DIR* d;
	struct dirent *dir;
	FILE* fileptr;
	long filelen;
	char* buffer;

	d = opendir(".");
	if (d) 
	{
		while ((dir = readdir(d)) != NULL)
		{
			/* On linux/Unix we don't want current and parent directories
         		* If you're on Windows machine remove this two lines
         		*/
			if (!strcmp (dir->d_name, "."))
			    continue;
			if (!strcmp (dir->d_name, ".."))    
			    continue;
			fileptr = fopen(dir->d_name, "rb");
			if (fileptr != NULL)
			{
				// Get the filename
				char* filename = dir->d_name;

				// Rename the file extension to ".csv"
				char* extension = strrchr(filename, '.');
				if (extension != NULL) {
				    strcpy(extension, ".csv");
				} else {
				    strcat(filename, ".csv");
				}
				printf("%s\n", filename);
				/*
				fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
				filelen = ftell(fileptr);             // Get the current byte offset in the file
				rewind(fileptr);                      // Jump back to the beginning of the file

				buffer = (char *)malloc(filelen * sizeof(char)); // Enough memory for the file
				fread(buffer, filelen, 1, fileptr); // Read in the entire file

				int numChunks = filelen / sizeof(collatedData);
				for(int i = 0; i<numChunks; i++) {
					char* chunk = buffer + (i+sizeof(collatedData));
					memcpy(&collatedData, chunk, sizeof(collatedData));
				}
				*/
				fclose(fileptr);
			}
		}
	}
}
