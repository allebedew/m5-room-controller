
#include <Arduino.h>

int32_t arc_x(int32_t x, int32_t a, int32_t r);
int32_t arc_y(int32_t y, int32_t a, int32_t r);
void draw_arc_arrow(TFT_eSprite sprite, int32_t x, int32_t y, int32_t a, int32_t rx, int32_t ry, uint32_t color, int32_t w,
    int32_t h, bool inside);
void fillArc(TFT_eSprite sprite, int x, int y, int start_angle, int seg_count, int rx, int ry, int w, unsigned int colour);
unsigned int brightness(unsigned int colour, int brightness);
unsigned int rainbow(byte value);