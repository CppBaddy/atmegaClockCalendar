/*
 * RotaryEncoder.hpp
 *
 *  Created on: 2020-04-12
 *      Author: yulay
 */

#ifndef ROTARYENCODER_HPP_
#define ROTARYENCODER_HPP_

#include <stdint.h>

namespace yr
{
    enum
    {
        None                = 0,
        Btn_Released        = -1,
        Btn_Pressed         = 1,
        Btn_PressedLong     = 2 | Btn_Pressed,
        Increment           = 1,
        Decrement           = -1,
    };

    class RotaryEncoder
    {
    public:
        RotaryEncoder ()
        : mInput(0)
        , mButton(0)
        , mMax(99)
        , mValue(0)
        , mPrevInput(0)
        {}

        ~RotaryEncoder () = default;

        void Update();

        void inc() { ++mInput; }
        void dec() { --mInput; }

        void pressed() { mButton = Btn_Pressed; }
        void longPressed() { mButton = Btn_PressedLong; }

        int8_t isPressed() { return mButton; }

        void setValue(uint8_t v) { mValue = (mMax < v) ? mMax : v; }
        uint8_t getValue() const { return mValue; }

        void setMax(uint8_t v) { mMax = v; if(mMax < mValue) mValue = mMax; }

        uint8_t getButton();

    private:
        //current hw encoder tracking
        volatile uint8_t mInput;
        volatile uint8_t mButton;

        //current software encoder state
        uint8_t mMax;
        int8_t mValue;
        int8_t mPrevInput;
    };

} /* namespace yr */

#endif /* ROTARYENCODER_HPP_ */
