//------------------------------------------------------------------------------
// This is Open source software. You can place this code on your site, but don't
// forget a link to my YouTube-channel: https://www.youtube.com/channel/UChButpZaL5kUUl_zTyIDFkQ
// Это программное обеспечение распространяется свободно. Вы можете размещать
// его на вашем сайте, но не забудьте указать ссылку на мой YouTube-канал 
// "Электроника в объектике" https://www.youtube.com/channel/UChButpZaL5kUUl_zTyIDFkQ
// Автор: Надыршин Руслан / Nadyrshin Ruslan
//------------------------------------------------------------------------------
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "display.h"
#include "fonts/font.h"

#include "st7920.h"

uint8_t sFrameBuf[(DISPLAY_Width * DISPLAY_Height) / 8];

// инициализирует 1-цветый дисплей
void display_Init()
{
    ST7920_Init(DISPLAY_Width, DISPLAY_Height);
}

// заполняет буфер кадра значением FillValue
void display_FillScreenbuff (uint8_t value)
{
    memset (sFrameBuf, value, sizeof(sFrameBuf));
}

// обновляет состояние индикаторов в соответствии с буфером кадра disp1color_buff
void display_UpdateFromBuff (void)
{
    ST7920_DisplayFullUpdate (sFrameBuf, sizeof(sFrameBuf));
}

// выводит на дисплей форматированную строку
uint8_t display_printf (int16_t X, int16_t Y, uint8_t FontID, const char *args, ...)
{
    char StrBuff[60];

    va_list ap;
    va_start(ap, args);
    char len = vsnprintf (StrBuff, sizeof(StrBuff), args, ap);
    va_end(ap);

    display_DrawString (X, Y, FontID, (uint8_t *) StrBuff);

    return len;
}

// устанавливает состояние 1 пикселя дисплея
void display_DrawPixel (int16_t x, int16_t y, uint8_t value)
{
    // Проверяем, находится ли точка в поле отрисовки дисплея
    if ((x >= DISPLAY_Width) || (y >= DISPLAY_Height) || (x < 0) || (y < 0))
        return;

    uint16_t byteIdx = y >> 3;

    uint8_t bitIdx = y - (byteIdx << 3); // Высота относительно строки байт (0<=Y<=7)

    byteIdx *= DISPLAY_Width;

    byteIdx += x;

    if (value)
        sFrameBuf[byteIdx] |= (1 << bitIdx);
    else
        sFrameBuf[byteIdx] &= ~(1 << bitIdx);
}

// рисует прямую линию в буфере кадра дисплея
void display_Line (int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t val)
{
    const int16_t deltaX = abs (x2 - x1);
    const int16_t deltaY = abs (y2 - y1);
    const int16_t signX = x1 < x2 ? 1 : -1;
    const int16_t signY = y1 < y2 ? 1 : -1;

    int16_t error = deltaX - deltaY;

    display_DrawPixel (x2, y2, val);

    while (x1 != x2 || y1 != y2)
    {
        display_DrawPixel (x1, y1, val);
        const int16_t error2 = error * 2;

        if (error2 > -deltaY)
        {
            error -= deltaY;
            x1 += signX;
        }
        if (error2 < deltaX)
        {
            error += deltaX;
            y1 += signY;
        }
    }
}

void display_FillRectangle (int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t fill)
{
    for(uint8_t i = y1; i <= y2; ++i)
    {
        display_Line(x1, i, x2, i, fill);
    }
}

// рисует прямоугольник в буфере кадра дисплея
void display_DrawRectangle (int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
    display_DrawLine (x1, y1, x1, y2);
    display_DrawLine (x2, y1, x2, y2);
    display_DrawLine (x1, y1, x2, y1);
    display_DrawLine (x1, y2, x2, y2);
}

// рисует окружность в буфере кадра дисплея. x0 и y0 - координаты центра окружности
void display_DrawCircle (int16_t x0, int16_t y0, int16_t radius)
{
    int x = 0;
    int y = radius;
    int delta = 1 - 2 * radius;
    int error = 0;

    while (y >= 0)
    {
        display_DrawPixel (x0 + x, y0 + y, 1);
        display_DrawPixel (x0 + x, y0 - y, 1);
        display_DrawPixel (x0 - x, y0 + y, 1);
        display_DrawPixel (x0 - x, y0 - y, 1);
        error = 2 * (delta + y) - 1;

        if (delta < 0 && error <= 0)
        {
            ++x;
            delta += 2 * x + 1;
            continue;
        }

        error = 2 * (delta - x) - 1;

        if (delta > 0 && error > 0)
        {
            --y;
            delta += 1 - 2 * y;
            continue;
        }

        ++x;

        delta += 2 * (x - y);

        --y;
    }
}

// вывода символа Char на дисплей. Возвращает ширину выведенного символа
uint8_t display_DrawChar (int16_t x, int16_t y, uint8_t fontId, uint8_t c)
{
    // Указатель на подтабличку конкретного символа шрифта
    uint8_t *charTable = font_GetFontStruct (fontId, c);

    uint8_t charWidth = charTable[0];    // Ширина символа
    uint8_t charHeight = charTable[1];  // Высота символа

    charTable += 2; //skip width and height

    if (fontId == FONTID_6X8M)
    {
        for (uint8_t row = 0; row < charHeight; ++row)
        {
            uint8_t chRow = charTable[row];
            uint8_t mask = 0b10000000;

            for (uint8_t col = 0; col < charWidth; ++col)
            {
                display_DrawPixel (x + col, y, chRow & mask);

                mask >>= 1;
            }

            ++y;
        }
    }

    return charWidth;
}

uint8_t display_DrawInvertedChar (int16_t x, int16_t y, uint8_t fontId, uint8_t c)
{
    // Указатель на подтабличку конкретного символа шрифта
    uint8_t *charTable = font_GetFontStruct (fontId, c);

    uint8_t charWidth = charTable[0];    // Ширина символа
    uint8_t charHeight = charTable[1];  // Высота символа

    charTable += 2; //skip width and height

    if (fontId == FONTID_6X8M)
    {
        for (uint8_t row = 0; row < charHeight; ++row)
        {
            uint8_t chRow = ~(charTable[row]);
            uint8_t mask = 0b10000000;

            for (uint8_t col = 0; col < charWidth; ++col)
            {
                display_DrawPixel (x + col, y, chRow & mask);

                mask >>= 1;
            }

            ++y;
        }
    }

    return charWidth;
}

// вывода текста из строки Str на дисплей
void display_DrawString (int16_t x, int16_t y, uint8_t fontId, const uint8_t* s)
{
    uint8_t strHeight = 8; // Высота символов в пикселях для перехода на следующую строку
    bool inverted = false;

    // Вывод строки
    for(int16_t i=x; *s; ++s)
    {
        switch (*s)
        {
            case '\n':  // Переход на следующую строку
                y += strHeight;
                break;
            case '\r':  // Переход в начало строки
                i = x;
                break;
            case '\b':
                inverted = !inverted;
                break;
            default:    // Отображаемый символ
                if(inverted)
                {
                    i += display_DrawInvertedChar (i, y, fontId, *s);
                }
                else
                {
                    i += display_DrawChar (i, y, fontId, *s);
                }

                strHeight = font_GetCharHeight (font_GetFontStruct (fontId, *s));

                break;
        }
    }
}

