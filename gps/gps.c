/*
 * gps.c
 *
 *  Created on: Nov 15, 2019
 *      Author: Bulanov Konstantin
 *
 *  Contact information
 *  -------------------
 *
 * e-mail   :  leech001@gmail.com
 */

/*
 * |---------------------------------------------------------------------------------
 * | Copyright (C) Bulanov Konstantin,2019
 * |
 * | This program is free software: you can redistribute it and/or modify
 * | it under the terms of the GNU General Public License as published by
 * | the Free Software Foundation, either version 3 of the License, or
 * | any later version.
 * |
 * | This program is distributed in the hope that it will be useful,
 * | but WITHOUT ANY WARRANTY; without even the implied warranty of
 * | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * | GNU General Public License for more details.
 * |
 * | You should have received a copy of the GNU General Public License
 * | along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * |---------------------------------------------------------------------------------
 */
#include "gps.h"

uint8_t rx_data = 0;
uint8_t rx_buffer[GPSBUFSIZE];
uint8_t rx_index = 0;

GPS_t GPS;
float speed_k;
char ns,ew, units;
int lock, satelites;
float nmea_latitude, nmea_longitude, hdop;
#if (GPS_DEBUG == 1)
void GPS_print(char *data){
	for(int i=0; i<GPSBUFSIZE; i++) {
		printf("%d/n", data[i]);
	}
}
#endif

void GPS_Init()
{
	HAL_UART_Receive_IT(GPS_USART, &rx_data, 1);
}

int GPS_validate(char *nmeastr){
	char check[3];
	char checkcalcstr[3];
	int i;
	int calculated_check;

	i=0;
	calculated_check=0;

	// check to ensure that the string starts with a $
	if(nmeastr[i] == '$')
		i++;
	else
		return 0;

	//No NULL reached, 75 char largest possible NMEA message, no '*' reached
	while((nmeastr[i] != 0) && (nmeastr[i] != '*') && (i < 75)){
		calculated_check ^= nmeastr[i];// calculate the checksum
		i++;
	}

	if(i >= 75){
		return 0;// the string was too long so return an error
	}

	if (nmeastr[i] == '*'){
		check[0] = nmeastr[i+1];    //put hex chars in check string
		check[1] = nmeastr[i+2];
		check[2] = 0;
	}
	else
		return 0;// no checksum separator found there for invalid

	sprintf(checkcalcstr,"%02X",calculated_check);
	return((checkcalcstr[0] == check[0])
			&& (checkcalcstr[1] == check[1])) ? 1 : 0 ;
}

int GPS_parse(char *GPSstrParse){

	if(!strncmp(GPSstrParse, "$GPGGA", 6)){
		printf(GPSstrParse);
		printf("\n");
	    	if (sscanf(GPSstrParse, "$GPGGA,%f,%f,%c,%f,%c,%d,%d,%f,%f,%c", &GPS.utc_time, &nmea_latitude, &ns, &nmea_longitude, &ew, &lock, &satelites, &hdop, &GPS.altitude, &units) >= 1){
	    		GPS.dec_latitude = GPS_nmea_to_dec(nmea_latitude, ns);
	    		GPS.dec_longitude = GPS_nmea_to_dec(nmea_longitude, ew);
	    		printf("measured values:\n"
									"\t%0.5f latitude\n"
									"\t%0.5f longitude\n"
									"\t%0.5f feet altitude\n"
									"\t%0.2f time\n"
									"\t%.2lu date\n",
									GPS.dec_latitude, ns, GPS.dec_longitude, ew, GPS.altitude, GPS.utc_time, GPS.date
				);
	    		return 1;
	    	}
	    }
	else {
		return 0;
	}
}

float GPS_nmea_to_dec(float deg_coord, char nsew) {
	int degree = (int)(deg_coord/100);
	float minutes = deg_coord - degree*100;
	float dec_deg = minutes / 60;
	float decimal = degree + dec_deg;
	if (nsew == 'S' || nsew == 'W') { // return negative
		decimal *= -1;
	}
	return decimal;
}


