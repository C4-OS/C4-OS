#ifndef _C4OS_STUBBY_PPM_H
#define _C4OS_STUBBY_PPM_H 1
#include <stubbywm/stubbywm.h>

typedef struct ppm {
	uint8_t version;
	unsigned width;
	unsigned height;
	uint8_t *pixbuf;
} ppm_t;

void ppm_load(ppm_t *ppm, void *data);
void ppm_draw(ppm_t *ppm, wm_t *wm, stubby_point_t coord);

#endif
