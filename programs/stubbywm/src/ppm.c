#include <c4rt/c4rt.h>
#include <c4rt/stublibc.h>
#include <c4rt/interface/framebuffer.h>
#include <stubbywm/stubbywm.h>
#include <stubbywm/ppm.h>

// XXX: this does basically no validation on the file and assumes everything
//      is where it should be, this would be trivially exploited if it's
//      ever used to load untrusted data

// TODO: merge this into stublibc
static int atoi( const char *s ){
	int ret = 0;

	for ( unsigned i = 0; s[i]; i++ ){
		ret *= 10;
		ret += s[i] - '0';
	}

	return ret;
}

static unsigned parse_text(char **buf) {
	char temp[32];

	// handle comments
	while (**buf == '#') {
		size_t n = strcspn(*buf, "\n");
		*buf += n + 1;
	}

	// TODO: is strcspn returning the right result?
	size_t n = strcspn(*buf, " \n");
	C4_ASSERT(n < sizeof(temp));
	strlcpy(temp, *buf, n + 1);
	*buf += n + 1;

	return atoi(temp);
}

void ppm_load(ppm_t *ppm, void *data) {
	uint8_t *buf = data;

	ppm->version = buf[1];
	buf += 3;
	// we can only load ppm version 6 files for now
	C4_ASSERT(ppm->version == '6');

	ppm->width  = parse_text((char **)&buf);
	ppm->height = parse_text((char **)&buf);
	// parse the maximum pixel value field, and dutifully ignore it
	parse_text((char**)&buf);
	ppm->pixbuf = buf;
}

void ppm_draw(ppm_t *ppm, wm_t *wm, stubby_point_t coord) {
	for (unsigned y = 0; y < ppm->height; y++) {
		for (unsigned x = 0; x < ppm->width; x++) {
			uint8_t *data = ppm->pixbuf + ((y * ppm->width) + x) * 3;

			uint8_t red   = data[0];
			uint8_t green = data[1];
			uint8_t blue  = data[2];
			uint32_t pixel = (red << 16) | (green << 8) | blue;

			if (pixel == 0x00ff00) {
				// XXX: fully green pixels are treated as transparent
				continue;
			}

			draw_pixel(wm, x + coord.x, y + coord.y, pixel);
		}
	}
}
