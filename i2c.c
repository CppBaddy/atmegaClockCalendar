/* I2C (TwoWire) AVR library
 *
 * Copyright (C) 2015-2017 Sergey Denisov.
 * Rewritten by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public Licence
 * as published by the Free Software Foundation; either version 3
 * of the Licence, or (at your option) any later version.
 */
#include "i2c.h"



void i2c_init(void)
{
    TWBR = ((F_CPU / TWI_FREQ) - 16) / 2;

    //defaults: TWBR = 0, TWPS1 = TWPS0 = 0 -> TWI clock = FCPU / 16
    // CPU @ 2MHz, twi clock 125kHz
}

bool i2c_start_condition(void)
{
	TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);

	while (i2c_busy());

	return TW_START == i2c_status() || TW_REP_START == i2c_status();
}

void i2c_stop_condition(void)
{
	TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);
}

void i2c_send_byte(unsigned char byte)
{
	TWDR = byte;
    TWCR = _BV(TWINT) | _BV(TWEN);

    while (i2c_busy());
}

void i2c_send_packet(unsigned char value, unsigned char address)
{
	i2c_start_condition();
	i2c_send_byte(address);
	i2c_send_byte(value);
	i2c_stop_condition();
}

uint8_t i2c_recv_byte(void)
{
	TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWEA);

	while (i2c_busy());

	return TWDR;
}

uint8_t i2c_recv_last_byte(void)
{
	TWCR = _BV(TWINT) | _BV(TWEN);

	while (i2c_busy());

    return TWDR;
}
