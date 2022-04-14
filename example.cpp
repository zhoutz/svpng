#include "svpng.hpp"

void test_rgb() {
  unsigned char rgb[256 * 256 * 3], *p = rgb;
  unsigned x, y;
  for (y = 0; y < 256; y++)
    for (x = 0; x < 256; x++) {
      *p++ = (unsigned char)x; /* R */
      *p++ = (unsigned char)y; /* G */
      *p++ = 128;              /* B */
    }
  svpng("./rgb.png", 256, 256, rgb, false);
}

void test_rgba() {
  unsigned char rgba[256 * 256 * 4], *p = rgba;
  unsigned x, y;
  for (y = 0; y < 256; y++)
    for (x = 0; x < 256; x++) {
      *p++ = (unsigned char)x;             /* R */
      *p++ = (unsigned char)y;             /* G */
      *p++ = 128;                          /* B */
      *p++ = (unsigned char)((x + y) / 2); /* A */
    }
  svpng("./rgba.png", 256, 256, rgba, true);
}

void test_high_reso(unsigned width, unsigned height) {
  unsigned char* rgba = (unsigned char*)malloc(width * height * 4);
  unsigned char* p = rgba;
  for (unsigned y = 0; y < height; ++y) {
    for (unsigned x = 0; x < width; ++x) {
      *p++ = (unsigned char)(2 * x + 2 * y); /* R */
      *p++ = (unsigned char)(3 * x + 5 * y); /* G */
      *p++ = (unsigned char)(5 * x + 2 * y); /* B */
      *p++ = (unsigned char)(x + y);         /* A */
    }
  }
  svpng("./wallpaper.png", width, height, rgba, true);
  free(rgba);
}

int main() {
  test_rgb();
  test_rgba();
  test_high_reso(3840, 2160);

  return 0;
}
