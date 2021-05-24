/*
 * digits.h
 *
 * Модуль для отрисовки цифр часов и анимации переходов
 *
 * Author: Погребняк Дмитрий (Pogrebnyak Dmitry, http://aterlux.ru/)
 */ 


#ifndef DIGITS_H_
#define DIGITS_H_

#include <avr/io.h>

#define DIGIT_EMPTY 255
#define DIGIT_TIME_SEPARATOR 10
#define DIGIT_DATE_SEPARATOR 11

#define DIGIT_WIDTH 16

/* Выбирает начертание цифр */
void digits_select_font(uint8_t font);

/* Возвращает код текущего начертания цифр */
uint8_t digits_current_font();

/* Выводит указанную цифру в указанной горизонтальной позиции */
uint8_t digit_output(uint8_t d, uint8_t x);
uint8_t digit_output_inv(uint8_t d, uint8_t x);

/* Помещает две последние десятичные цифры числа num в два байта, начиная с p_buf.
 * Если leading_zero == 0, то для чисел меньше 10 на первой позиции  помещает DIGIT_EMPTY*/
uint8_t two_digits(uint8_t num, uint8_t * p_buf, uint8_t leading_zero);


#endif /* DIGITS_H_ */
