#ifndef __OLED_FONT_H
#define __OLED_FONT_H

#include <stdint.h>

/*
 * 5x7 ASCII font.
 * Supported clearly:
 *   space, 0~9, A~Z, a~z(mapped to A~Z), ':', '-', '_', '.', '/'
 * Unsupported characters are displayed as '?'.
 */

const uint8_t *OLED_Font_Get5x7(char ch);

#endif
