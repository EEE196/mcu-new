#ifndef TEST_H_
#define TEST_H_
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <usart.h>
#include <spi.h>

#include "fatfs.h"
#include "../pm2.5/sps30.h"
#include "../gps/gps.h"
#include "../lora/rfm95.h"
#include "../co2/scd30.h"
#include "../sd/fatfs_sd_card.h"
void test_sd( void );
void test_pm( void );
void test_lora( void );
void test_gps(void);
void test_co(void);


#endif /* TEST_H_ */
