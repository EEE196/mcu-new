#include <stdio.h>
#include <math.h>
uint32_t so_convert(uint32_t vgas, uint32_t vgas0, uint32_t vtemp, uint32_t vref) {
	float vgas_f = (float)vgas*vref/4095;
	float vgas0_f = (float)vgas0*vref/4095;
	float vtemp_f = (float)vtemp*vref/4095;
	//printf("%f Vgas\n", vgas_f);
	//printf("%f Vgas0\n", vgas0_f);
	//printf("%f Vtemp\n", vtemp_f);

	float M = 31.73 * 100 * (pow(10, -6));
	float cx = (vgas_f - vgas0_f)/M;
	//printf("%f\n", cx);
	long int lround(double cx);
	return cx;
}
