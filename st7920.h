//------------------------------------------------------------------------------
// This is Open source software. You can place this code on your site, but don't
// forget a link to my YouTube-channel: https://www.youtube.com/channel/UChButpZaL5kUUl_zTyIDFkQ
// Это программное обеспечение распространяется свободно. Вы можете размещать
// его на вашем сайте, но не забудьте указать ссылку на мой YouTube-канал 
// "Электроника в объектике" https://www.youtube.com/channel/UChButpZaL5kUUl_zTyIDFkQ
// Автор: Надыршин Руслан / Nadyrshin Ruslan
//------------------------------------------------------------------------------
#ifndef _ST7920_H
#define _ST7920_H

#include <stdint.h>

#define delay_ms(x)	_delay_ms(x)
#define delay_us(x)	_delay_us(x)


// Варианты интерфейса обмена с дисплеем
#define ST7920_IF_Parallel_4bit 0
#define ST7920_IF_Parallel_8bit 1
#define ST7920_IF_SPI           2


// Используемый для подключения к дисплею интерфейс
#define ST7920_IF               ST7920_IF_SPI
// Используется управление ножкой RESET
#define ST7920_RESET_Used       1


// Задержки в работе интерфейса дисплея в мкс
/// Параллельный 4бита
#define ST7920_ShortDelayUs_1   3
#define ST7920_ShortDelayUs_2   10
/// Параллельный 8бит
#define ST7920_ShortDelayUs_3   8
#define ST7920_ShortDelayUs_4   20


#if (ST7920_IF == ST7920_IF_SPI)
  #define ST7920_StartByte_RWmask       (1 << 2)
  #define ST7920_StartByte_RSmask       (1 << 1)

  #define ST7920_RS_CS_Used     1       // Можно установить в 0 если дисплей один и на CS лог. 1
#else
  #define ST7920_RS_CS_Used     1
#endif

//==============================================================================
// Настройки подключения к интерфейсу дисплея
//==============================================================================
// Сигнал RESET
#if (ST7920_RESET_Used)
  #define ST7920_RESET_Port     PORTB
  #define ST7920_RESET_DDR      DDRB
  #define ST7920_RESET_Mask     (1 << PB7)
#endif

// Сигнал RS (для параллельного интерфейса) / CS (для SPI)
#define ST7920_RS_CS_Port       PORTB
#define ST7920_RS_CS_DDR        DDRB
#define ST7920_RS_CS_Mask       (1 << DDB2)

#if (ST7920_IF != ST7920_IF_SPI)        // Параллельный интерфейс?
  // Сигнал RW (1 - чтение, 0 - запись)
  #define ST7920_RW_Port        PORTC
  #define ST7920_RW_DDR         DDRC
  #define ST7920_RW_Mask        (1 << 2)
  // Сигнал E (строб для параллельного интерфейса)
  #define ST7920_E_Port         PORTC
  #define ST7920_E_DDR          DDRC
  #define ST7920_E_Mask         (1 << 3)
  // Ноги данных параллельного интерфейса hd44780
  #define ST7920_Data_Port      PORTD
  #define ST7920_Data_DDR       DDRD
  #if (ST7920_IF == ST7920_IF_Parallel_4bit)
    #define ST7920_Data_Shift          0      // Битовый сдвиг параллельной шины влево по порту МК
  #endif
#endif
//==============================================================================
// Набор команд Basic
#define ST7920_CmdBasic_Clear                   0x01    // Очистка DDRAM
#define ST7920_CmdBasic_Home                    0x02    // Перевод курсора в начало дисплея
#define ST7920_CmdBasic_EntryMode               0x04    // Параметры автосдвига курсора и экрана
#define ST7920_CmdBasic_DisplayOnOff            0x08    // Управление дисплеем и курсором
#define ST7920_CmdBasic_CursorDisplayControl    0x10    // 
#define ST7920_CmdBasic_FunctionSet             0x20    // Выбор битности интерфейса и управление Extended Mode 
#define ST7920_CmdBasic_SetCGRAMaddr            0x40    // Установка адреса в CGRAM 
#define ST7920_CmdBasic_SetDDRAMaddr            0x80    // Установка адреса в DDRAM
// Набор команд Extended
#define ST7920_CmdExt_StandBy                   0x01    // Перевод в режим StandBy
#define ST7920_CmdExt_SelScrollOrRamAddr        0x02    // Выбор скрола либо адреса в памяти
#define ST7920_CmdExt_Reverse                   0x04    // Реверс одной из 4 строк в DDRAM
#define ST7920_CmdExt_FunctionSet               0x20    // Выбор битности интерфейса, управление Extended Mode и Graphic Mode
#define ST7920_CmdExt_SetIRAMOrSccrollAddr      0x40    // Установка адреса в IRAM или сдвиг скролла
#define ST7920_CmdExt_SetGDRAMAddr              0x80    // Установка адреса в GDRAM (память графического режима)
//==============================================================================


// Процедура инициализации дисплея
void ST7920_Init(uint8_t Width, uint8_t Height);
// Процедура заполняет буфер графического режима дисплея в соответствии с буфером pBuff
void ST7920_DisplayFullUpdate(uint8_t *pBuff, uint16_t BuffLen);

#endif
