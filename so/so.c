#include <stdio.h>
#include <math.h>
float so_convert(uint32_t vgas, uint32_t vgas0, uint32_t vtemp) {
	float vgas_f = (float)vgas*3.25/4095;
	float vgas0_f = (float)vgas0*3.25/4095;
	float vtemp_f = (float)vtemp*3.25/4095;
	//printf("%f Vgas\n", vgas_f);
	//printf("%f Vgas0\n", vgas0_f);
	//printf("%f Vtemp\n", vtemp_f);

	float M = 31.73 * 100 * (pow(10, -6));
	float cx = (vgas_f - vgas0_f)/M;

	return cx;
}
