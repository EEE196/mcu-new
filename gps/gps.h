/*
 * gps.h
 *
 *  Created on: Nov 15, 2019
 *      Author: Bulanov Konstantin
 */
#include <stdio.h>
#include <string.h>
#include <usart.h>

#define GPS_DEBUG	1
#define	GPS_USART	&huart1
#define GPSBUFSIZE  128       // GPS buffer size

typedef struct __attribute__((packed)) {


    // GGA - Global Positioning System Fixed Data
    float dec_longitude;
    float dec_latitude;
    float altitude;
    float utc_time;
    // RMC - Recommended Minimmum Specific GNS Data
    uint32_t date;
} GPS_t;

#if (GPS_DEBUG == 1)
void GPS_print(char *data);
#endif

extern uint8_t rx_data;
extern uint8_t rx_buffer[GPSBUFSIZE];
extern uint8_t rx_index;
extern GPS_t GPS;

void GPS_Init();
void GSP_USBPrint(char *data);
void GPS_print_val(char *data, int value);
int GPS_validate(char *nmeastr);
int  GPS_parse(char *GPSstrParse);
float GPS_nmea_to_dec(float deg_coord, char nsew);

