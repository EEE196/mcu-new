
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

void writeCollatedData(FILE* file, const CollatedData* data) {
    
    // Write GPS data
    fprintf(file, "%f,%f,%f,%d,", data->GPS_Data.nmea_longitude, data->GPS_Data.nmea_latitude, data->GPS_Data.utc_time, data->GPS_Data.date);

    // Write CO data
    fprintf(file, "%f,%d,%f,%f,", data->CO_Data.co2_ppm, data->PM_Data.SO_ppm, data->CO_Data.temperature, data->CO_Data.relative_humidity);

    // Write PM data
    fprintf(file, "%f,%f,%f,%f\n", data->PM_Data.mc_2p5, data->PM_Data.mc_10p0, data->PM_Data.nc_2p5, data->PM_Data.nc_10p0);
}

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

				fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
				filelen = ftell(fileptr);             // Get the current byte offset in the file
				rewind(fileptr);                      // Jump back to the beginning of the file

				buffer = (char *)malloc(filelen * sizeof(char)); // Enough memory for the file
				fread(buffer, filelen, 1, fileptr); // Read in the entire file

				int numChunks = filelen / sizeof(collatedData);

				// Write column names
            			FILE* dataFile = fopen(filename, "r");
				fprintf(dataFile, "Longitude E,Latitude N,UTC Time,Date,CO2 ppm,SO2 ppm,Temperature °,Relative Humidity %,PM2.5 ppm,PM10 ppm,NC2.5 #/cm^3,NC10 #/cm^3\n");
				for(int i = 0; i<numChunks; i++) {
					char* chunk = buffer + (i+sizeof(collatedData));
					memcpy(&collatedData, chunk, sizeof(collatedData));
					writeCollatedData(dataFile, &collatedData);
				}

				fclose(fileptr);
				fclose(dataFile);
			}
		}
	}
}
