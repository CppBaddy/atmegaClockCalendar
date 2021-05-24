//------------------------------------------------------------------------------
// This is Open source software. You can place this code on your site, but don't
// forget a link to my YouTube-channel: https://www.youtube.com/channel/UChButpZaL5kUUl_zTyIDFkQ
// Это программное обеспечение распространяется свободно. Вы можете размещать
// его на вашем сайте, но не забудьте указать ссылку на мой YouTube-канал 
// "Электроника в объектике" https://www.youtube.com/channel/UChButpZaL5kUUl_zTyIDFkQ
// Автор: Надыршин Руслан / Nadyrshin Ruslan
//------------------------------------------------------------------------------
#ifndef _DISP1COLOR_H
#define _DISP1COLOR_H

#include <stdint.h>

// Список поддерживаемых дисплеев (контроллеров дисплеев)
#define DISPTYPE_ssd1306        0       // OLED-дисплей с контроллером ssd1306
#define DISPTYPE_DMD_1color     1       // LED-матрица одноцветная на логике
#define DISPTYPE_st7920         2       // LCD-дисплей с контроллером st7920


// С каким типом дисплея будет работать модуль disp1color 
#define DISP1COLOR_type         DISPTYPE_st7920

// Размеры дисплея в пикселях
#define DISPLAY_Width        128
#define DISPLAY_Height       64


// инициализирует 1-цветый дисплей
void display_Init();

// управляет режимом Test дисплея
void display_TestMode(uint8_t TestOn);

// устанавливает яркость дисплея
void display_SetBrightness(uint8_t Value);

// заполняет буфер кадра значением FillValue
void display_FillScreenbuff(uint8_t FillValue);

// обновляет состояние индикаторов в соответствии с буфером кадра
void display_UpdateFromBuff(void);

void display_DrawPixel (int16_t X, int16_t Y, uint8_t v);

// рисует прямую линию в буфере кадра дисплея
void display_Line(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t val);

inline void display_DrawLine (int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
    display_Line(x1, y1, x2, y2, 1);
}
inline void display_CleanLine (int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
    display_Line(x1, y1, x2, y2, 0);
}

void display_FillRectangle (int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t fill);

// рисует прямоугольник в буфере кадра дисплея
void display_DrawRectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2);

// рисует окружность в буфере кадра дисплея. x0 и y0 - координаты центра окружности
void display_DrawCircle(int16_t x0, int16_t y0, int16_t radius);

// вывода символа Char на дисплей. Возвращает ширину выведенного символа
uint8_t display_DrawChar(int16_t X, int16_t Y, uint8_t FontID, uint8_t Char);

// вывода текста из строки Str на дисплей
void display_DrawString(int16_t X, int16_t Y, uint8_t FontID, const uint8_t *Str);

// выводит на дисплей форматированную строку
uint8_t display_printf(int16_t X, int16_t Y, uint8_t FontID, const char *args, ...);

#endif
