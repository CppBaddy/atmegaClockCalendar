/*
 * RotaryEncoder.cpp
 *
 *  Created on: 2020-04-12
 *      Author: yulay
 */

#include "RotaryEncoder.hpp"

#include <avr/interrupt.h>

namespace yr
{

    void RotaryEncoder::Update ()
    {
        uint8_t val = mInput;

        int8_t change = val - mPrevInput;

        if(change)
        {
            mValue += change;

            if(mValue > mMax)
            {
                mValue = 0;
            }
            else if(mValue < 0)
            {
                mValue = mMax;
            }

            mPrevInput = val;
        }
    }

    uint8_t RotaryEncoder::getButton()
    {
        cli();

        int8_t v = mButton;

        mButton = None;

        sei();

        return v;
    }

} /* namespace yr */
