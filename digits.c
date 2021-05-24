/*
 * digits.c
 *
 * Модуль для отрисовки цифр часов и анимации переходов
 *
 * Author: Погребняк Дмитрий (Pogrebnyak Dmitry, http://aterlux.ru/)
 */

#include "digits.h"
#include "digits_font.h"
#include "display.h"
#include <avr/pgmspace.h>


static uint8_t digits_font;

void digits_select_font (uint8_t font)
{
    if (font < DIGITS_FONTS_COUNT)
    {
        digits_font = font;
    }
}

uint8_t digits_current_font ()
{
    return digits_font;
}

uint8_t digit_output (uint8_t d, uint8_t x)
{
    if (x >= DISPLAY_Width)
    {
        return 1;
    }

//    if (d >= DIGITS_SYMBOLS_PER_FONT)
//    {
//        return display_fill_region (0, x, DIGITS_BLOCKS_PER_SYMBOL, DIGITS_COLS_PER_BLOCK, 0, 0);
//    }

    PGM_VOID_P pblocks = &digits_fonts[digits_font][d];

    uint8_t w = (x > (DISPLAY_Width - DIGITS_COLS_PER_BLOCK)) ? (DISPLAY_Width - x) : DIGITS_COLS_PER_BLOCK;

    for (uint8_t y = 0; y < DIGITS_BLOCKS_PER_SYMBOL; ++y)
    {
        PGM_VOID_P pdata = &digits_datablocks[pgm_read_byte(pblocks++)];

        uint8_t offsetY = y << 3; // y * 8

        for (uint8_t i = 0; i < w; ++i)
        {
            uint8_t col = pgm_read_byte(pdata++);

            uint8_t offsetX = x + i;

            for(uint8_t row = 0; row < 8; ++row)
            {
                display_DrawPixel (offsetX, offsetY + row, col & 1);
                col >>= 1;
            }
        }
    }

    return 1;
}

uint8_t digit_output_inv (uint8_t d, uint8_t x)
{
    if (x >= DISPLAY_Width)
    {
        return 1;
    }

//    if (d >= DIGITS_SYMBOLS_PER_FONT)
//    {
//        return display_fill_region (0, x, DIGITS_BLOCKS_PER_SYMBOL, DIGITS_COLS_PER_BLOCK, 0, 0);
//    }

    PGM_VOID_P pblocks = &digits_fonts[digits_font][d];

    uint8_t w = (x > (DISPLAY_Width - DIGITS_COLS_PER_BLOCK)) ? (DISPLAY_Width - x) : DIGITS_COLS_PER_BLOCK;

    for (uint8_t y = 0; y < DIGITS_BLOCKS_PER_SYMBOL; ++y)
    {
        PGM_VOID_P pdata = &digits_datablocks[pgm_read_byte(pblocks++)];

        uint8_t offsetY = y << 3; // y * 8

        for (uint8_t i = 0; i < w; ++i)
        {
            uint8_t col = ~(pgm_read_byte(pdata++));

            uint8_t offsetX = x + i;

            for(uint8_t row = 0; row < 8; ++row)
            {
                display_DrawPixel (offsetX, offsetY + row, col & 1);
                col >>= 1;
            }
        }
    }

    return 1;
}

uint8_t two_digits (uint8_t num, uint8_t* p_buf, uint8_t leading_zero)
{
    uint8_t r = num / 100;

    uint8_t d = (num % 100) / 10;

    if ((d == 0) && !leading_zero)
    {
        d = DIGIT_EMPTY;
    }

    num %= 10;

    *(p_buf++) = d;

    *p_buf = num;

    return r;
}

