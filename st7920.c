//------------------------------------------------------------------------------
// This is Open source software. You can place this code on your site, but don't
// forget a link to my YouTube-channel: https://www.youtube.com/channel/UChButpZaL5kUUl_zTyIDFkQ
// Это программное обеспечение распространяется свободно. Вы можете размещать
// его на вашем сайте, но не забудьте указать ссылку на мой YouTube-канал 
// "Электроника в объектике" https://www.youtube.com/channel/UChButpZaL5kUUl_zTyIDFkQ
// Автор: Надыршин Руслан / Nadyrshin Ruslan
//------------------------------------------------------------------------------
#include <avr/io.h>
//#include <inavr.h>
#include <util/delay.h>
#include "st7920.h"
#include "spim.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>


#if (ST7920_RS_CS_Used)
  #define ST7920_RS_CS_HIGH()           ST7920_RS_CS_Port |= ST7920_RS_CS_Mask 
  #define ST7920_RS_CS_LOW()            ST7920_RS_CS_Port &= ~ST7920_RS_CS_Mask
#else
  #define ST7920_RS_CS_HIGH()            
  #define ST7920_RS_CS_LOW()            
#endif

#if (ST7920_RESET_Used)
  #define ST7920_RESET_HIGH()           ST7920_RESET_Port |= ST7920_RESET_Mask 
  #define ST7920_RESET_LOW()            ST7920_RESET_Port &= ~ST7920_RESET_Mask
#else
  #define ST7920_RESET_HIGH()            
  #define ST7920_RESET_LOW()            
#endif

#define ST7920_RW_HIGH()                ST7920_RW_Port |= ST7920_RW_Mask 
#define ST7920_RW_LOW()                 ST7920_RW_Port &= ~ST7920_RW_Mask
#define ST7920_E_HIGH()                 ST7920_E_Port |= ST7920_E_Mask 
#define ST7920_E_LOW()                  ST7920_E_Port &= ~ST7920_E_Mask

#define ST7920_SetDATA_4bit(val)        {ST7920_Data_Port &= ~(0xF << ST7920_Data_Shift); ST7920_Data_Port |= (val << ST7920_Data_Shift);}
#define ST7920_SetDATA_8bit(val)        ST7920_Data_Port = val
#define ST7920_GetDATA_4bit()           ((ST7920_Data_Pin >> ST7920_Data_Shift) & 0xF)
#define ST7920_GetDATA_8bit()           ST7920_Data_Pin
#define ST7920_SetDATA_PinMode_In(mask)  ST7920_Data_DDR &= ~mask
#define ST7920_SetDATA_PinMode_Out(mask) ST7920_Data_DDR |= mask


static uint8_t sWidth;
static uint8_t sHeight;


//==============================================================================
// Процедура записи байта в дисплей
// - IsCmd - запись командного байта (иначе байта данных)
//==============================================================================
void ST7920_write(int8_t IsCmd, uint8_t Data)
{
  uint8_t StartByte = 0xF8;
  if (!IsCmd)
    StartByte |= ST7920_StartByte_RSmask;
  
  ST7920_RS_CS_HIGH();  // ChipSelect = 1
  
  // Отправляем Start Byte
  SPI_send8b(&StartByte, 1);
  // Отправляем High Byte
  StartByte = Data & 0xF0;
  SPI_send8b(&StartByte, 1);
  // Отправляем Low Byte
  StartByte = Data << 4;
  SPI_send8b(&StartByte, 1);
  
  ST7920_RS_CS_LOW();   // ChipSelect = 0
}
//==============================================================================


//==============================================================================
// Basic-команда очистки дисплея в текстовом режиме
//==============================================================================
void ST7920_Basic_Clear(void)
{
  ST7920_write(1, ST7920_CmdBasic_Clear);

  delay_us(1600);
}
//==============================================================================



//==============================================================================
// Basic-команда для установки курсора в текстовом режиме в начало
//==============================================================================
void ST7920_Basic_Home(void)
{
  ST7920_write(1, ST7920_CmdBasic_Home);

  delay_us(72);
}
//==============================================================================



//==============================================================================
// Basic-команда для установки параметров сдвига курсора и экрана
//==============================================================================
void ST7920_Basic_EntryMode(uint8_t ShiftOn, uint8_t MoveRight)
{
  uint8_t Data = ST7920_CmdBasic_EntryMode;
  if (ShiftOn)
    Data |= (1 << 0);
  if (MoveRight)
    Data |= (1 << 1);
  
  ST7920_write(1, Data);

  delay_us(72);
}
//==============================================================================


//==============================================================================
// Basic-команда для включения/отключения дисплея и управления отображением
// курсора в текстовом режиме
//==============================================================================
void ST7920_Basic_DisplayOnOff(uint8_t DisplayOn, uint8_t CursorOn, uint8_t BlinkOn)
{
  uint8_t Data = ST7920_CmdBasic_DisplayOnOff;

  if (DisplayOn)
    Data |= (1 << 2);
  if (CursorOn)
    Data |= (1 << 1);
  if (BlinkOn)
    Data |= (1 << 0);
  
  ST7920_write(1, Data);
  
  delay_us(72);
}
//==============================================================================


//==============================================================================
// Basic-команда для установки параметров сдвига курсора
//==============================================================================
void ST7920_Basic_CursorDisplayControl(uint8_t DisplayMoveRight, uint8_t CursorMoveRight)
{
  uint8_t Data = ST7920_CmdBasic_CursorDisplayControl;

  if (DisplayMoveRight)
    Data |= (1 << 3);
  if (CursorMoveRight)
    Data |= (1 << 2);

  ST7920_write(1, Data);

  delay_us(72);
}
//==============================================================================


//==============================================================================
// Basic-команда для управления текущим режимом (Basic-Extended).
// Также она устанавливает битность параллельного интерфейса.
//==============================================================================
void ST7920_Basic_FunctionSet(uint8_t ExtendedMode)
{
  uint8_t Data = ST7920_CmdBasic_FunctionSet;
  
  if (ExtendedMode)
    Data |= (1 << 2);
  
  ST7920_write(1, Data);

  delay_us(72);
}
//==============================================================================


//==============================================================================
// Basic-команда установки указателя в CGRAM
//==============================================================================
void ST7920_Basic_SetCGRAMaddr(uint8_t Addr)
{
  uint8_t Data = ST7920_CmdBasic_SetCGRAMaddr;
  Data |= (Addr & 0x3F);
  ST7920_write(1, Data);

  delay_us(72);
}
//==============================================================================


//==============================================================================
// Basic-команда установки указателя в DDRAM
//==============================================================================
void ST7920_Basic_SetDDRAMaddr(uint8_t Addr)
{
  uint8_t Data = ST7920_CmdBasic_SetDDRAMaddr;
  Data |= (Addr & 0x3F);
  ST7920_write(1, Data);

  delay_us(72);
}
//==============================================================================


//==============================================================================
// Extended-команда перевода в режим сна
//==============================================================================
void ST7920_Ext_StandBy(void)
{
  ST7920_write(1, ST7920_CmdExt_StandBy);

  delay_us(72);
}
//==============================================================================


//==============================================================================
// Extended-команда выбирает указатель на IRAM или Scroll-адрес
//==============================================================================
void ST7920_Ext_SelScrollOrRamAddr(uint8_t SelectScroll)
{
  uint8_t Data = ST7920_CmdExt_SelScrollOrRamAddr;
  
  if (SelectScroll)
    Data |= 0x01;
      
  ST7920_write(1, Data);

  delay_us(72);
}
//==============================================================================


//==============================================================================
// Extended-команда включает инверсию 1 из 4 строк. Повторный вызов отключает инверсию
//==============================================================================
void ST7920_Ext_Reverse(uint8_t Row)
{
  uint8_t Data = ST7920_CmdExt_Reverse;
  Data |= (Row & 0x03);
  ST7920_write(1, Data);

  delay_us(72);
}
//==============================================================================


//==============================================================================
// Extended-команда для управления текущим режимом (Basic-Extended), управления
// графическим режимом. Также она устанавливает битность параллельного интерфейса.
//==============================================================================
void ST7920_Ext_FunctionSet(uint8_t ExtendedMode, uint8_t GraphicMode)
{
  uint8_t Data = ST7920_CmdExt_FunctionSet;
  
#if ((ST7920_IF == ST7920_IF_Parallel_8bit) || (ST7920_IF == ST7920_IF_SPI))
  Data |= (1 << 4);
#endif
  
  if (ExtendedMode)
    Data |= (1 << 2);
  
  if (GraphicMode)
    Data |= (1 << 1);
  
  ST7920_write(1, Data);

  delay_us(72);
}
//==============================================================================


//==============================================================================
// Extended-команда установки указателя в IRAM или Scroll-адреса
//==============================================================================
void ST7920_Ext_SetIRAMOrSccrollAddr(uint8_t Addr)
{
  uint8_t Data = ST7920_CmdExt_SetIRAMOrSccrollAddr;
  Data |= (Addr & 0x3F);
  ST7920_write(1, Data);

  delay_us(72);
}
//==============================================================================


//==============================================================================
// Extended-команда установки указателя в буфере кадра графического режима
//==============================================================================
void ST7920_Ext_SetGDRAMAddr(uint8_t VertAddr, uint8_t HorizAddr)
{
  uint8_t Data = ST7920_CmdExt_SetGDRAMAddr;
  Data |= (VertAddr & 0x7F);
  ST7920_write(1, Data);
  
  Data = ST7920_CmdExt_SetGDRAMAddr;
  Data |= (HorizAddr & 0x0F);
  ST7920_write(1, Data);

  delay_us(72);
}
//==============================================================================


//==============================================================================
// Функция возвращает байт - горизонтальную строку пикселей из исходного буфера 
// кадра графической библиотеки
//==============================================================================
uint8_t ST7920_GetHorizontalByte(uint8_t *buff, uint8_t row, uint8_t col)
{
    // Считаем байтовый (стартовый) индекс в массиве исходного буфера кадра
    uint16_t byteIdx = (row >> 3) * sWidth;         // Индекс в исходном буфере начала строки
    byteIdx += (col << 3);

    // Определяем битовую маску, выделяющую строку из 8 пикселей в исходном буфере кадра
    uint8_t bitMask = row % 8;
    bitMask = (1 << bitMask);

    uint8_t byte = 0;

    // Извлекаем 8 бит
    for (uint8_t bit = 0; bit < 8; ++bit)
    {
        if (buff[byteIdx + bit] & bitMask)
        {
            byte |= (1 << (7 - bit));
        }
    }

    return byte;
}
//==============================================================================


//==============================================================================
// Процедура заполняет буфер графического режима дисплея в соответствии с буфером pBuff
//==============================================================================
void ST7920_DisplayFullUpdate(uint8_t *pBuff, uint16_t BuffLen)
{
  for (uint8_t Row = 0; Row < 32; ++Row)
  {
    // Встаём на начало очередной строки в буфере дисплея
    ST7920_Ext_SetGDRAMAddr(Row, 0);
    
    // Выводим строку высотой 1 пиксель в верхнюю половину дисплея
    for (uint8_t Col = 0; Col < 16; ++Col)
      ST7920_write(0, ST7920_GetHorizontalByte(pBuff, Row, Col));

    // Выводим строку высотой 1 пиксель в нижнюю половину дисплея
    for (uint8_t Col = 0; Col < 16; ++Col)
      ST7920_write(0, ST7920_GetHorizontalByte(pBuff, Row + 32, Col));
  }
}

//void ST7920_DisplayFullUpdate (uint8_t *buff, uint16_t size)
//{
//    for (uint8_t row = 0; row < 32; ++row)
//    {
//        // Встаём на начало очередной строки в буфере дисплея
//        ST7920_Ext_SetGDRAMAddr(row, 0);
//
//        uint16_t offset = row * 16;
//
//        // Выводим строку высотой 1 пиксель в верхнюю половину дисплея
//        for (uint8_t col = 0; col < 16; ++col)
//        {
//            ST7920_write(0, buff[offset + col]);
//        }
//
//        offset += 32 * 16;
//
//        // Выводим строку высотой 1 пиксель в нижнюю половину дисплея
//        for (uint8_t col = 0; col < 16; ++col)
//        {
//            ST7920_write(0, buff[offset + col]);
//        }
//    }
//}
//==============================================================================


//==============================================================================
// Процедура настройки ножек микроконтроллера для обмена с ST7920
//==============================================================================
void ST7920_GPIO_init(void)
{
  ST7920_RESET_HIGH();
  ST7920_RS_CS_LOW();

  // Настраиваем все используемые ноги как выходы
  
  // Сигнал RESET
  ST7920_RESET_DDR |= ST7920_RESET_Mask;
  
  // Сигнал RS (для параллельного интерфейса) / CS (для SPI)
  ST7920_RS_CS_DDR |= ST7920_RS_CS_Mask;
}
//==============================================================================


//==============================================================================
// Процедура отправляет команды инициализации дисплея в текстовом режиме
//==============================================================================
void ST7920_InitInTextMode(void)
{
  // Повторная инициализация контроллера в режиме Basic
  ST7920_Basic_FunctionSet(0);
  // Включаем дисплей без курсора
  ST7920_Basic_DisplayOnOff(1, 0, 0);
  // Очищаем текстовый буфер кадра
  ST7920_Basic_Clear();
  // Включаем автосдвиг курсора вправо
  ST7920_Basic_EntryMode(0, 1);
}
//==============================================================================


//==============================================================================
// Процедура отправляет команды инициализации дисплея в графическом режиме
//==============================================================================
void ST7920_InitInGraphMode(void)
{
  // Переключаем контроллер дисплея из режима Basic в Extended
  ST7920_Basic_FunctionSet(1);
  // Включаем графический режим
  ST7920_Ext_FunctionSet(1, 1);
}
//==============================================================================


//==============================================================================
// Процедура инициализации дисплея
//==============================================================================
void ST7920_Init(uint8_t w, uint8_t h)
{
  sWidth = w;
  sHeight = h;

  // Инициализация ножек интерфейса управления матрицами
  ST7920_GPIO_init();

  // Инициализация интерфейса SPI
  spim_init();
  
  // Задержка после подачи питания
  delay_ms(40);
  
  // Сброс дисплея
  ST7920_RESET_LOW();
  delay_ms(10);
  ST7920_RESET_HIGH();
  
  ST7920_Basic_FunctionSet(0);

  //ST7920_InitInTextMode();    // Инициализация дисплея в текстовом режиме
  ST7920_InitInGraphMode();     // Инициализация дисплея в графическом режиме
}
//==============================================================================
