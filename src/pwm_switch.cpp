#include "pwm_switch.hpp"

uint8_t PWMSwitch::mNextFreePWMChannel = 0;

uint8_t PWMSwitch::getNextPWMChannel()
{
    return mNextFreePWMChannel++;
}

void PWMSwitch::setup()
{
    ledcSetup(this->mPWMChannel, PWMSwitch::PWM_FREQ, PWMSwitch::RESOLUTION_BITS);
    ledcAttachPin(this->mPWMPin, mPWMChannel);
}

const String &PWMSwitch::getName() const
{
    return this->mName;
}

void PWMSwitch::setOutput(bool on)
{
    if (on && this->mDutyCycle != 0)
    {
        ledcWrite(this->mPWMChannel, this->mDutyCycle);
    }
    else
    {
        ledcWrite(this->mPWMChannel, 0);
    }
    this->mOutputEnable = on;
}

bool PWMSwitch::getOutput() const
{
    return this->mOutputEnable;
}

void PWMSwitch::setValue(uint8_t percentage)
{
    if (percentage > 100)
    {
        percentage = 100;
    }
    this->mDutyCycle = (PWMSwitch::MAX_VALUE * percentage) / 100;
    if (this->mDutyCycle == 0)
    { // Turn off to avoid leak current
        this->setOutput(false);
    }
    else if (this->getOutput())
    {
        ledcWrite(this->mPWMChannel, this->mDutyCycle);
    }
}

uint8_t PWMSwitch::getValue() const
{
    return (this->mDutyCycle * 100) / PWMSwitch::MAX_VALUE;
}

void PWMSwitch::measurePotentiometerSetValue()
{
    uint16_t sensor_value = analogRead(this->mPotentiometerPin);

    uint8_t pwm_value = (100 * sensor_value) / PWMSwitch::MAX_ANALOG_IN;

    uint16_t diff = 0;
    if (pwm_value > this->mLastMeasuredChange_perc)
    {
        diff = pwm_value - this->mLastMeasuredChange_perc;
    }
    else
    {
        diff = this->mLastMeasuredChange_perc - pwm_value;
    }
    if (diff > PWMSwitch::CHANGE_PERC)
    {
        this->setValue(pwm_value);
        this->mLastMeasuredChange_perc = pwm_value;
        // If value is below CHANGE_PERC turn it off
        if (pwm_value < PWMSwitch::CHANGE_PERC)
        {
            if (this->getOutput() == true)
            {
                this->setOutput(false);
            }
        }
        else
        {
            if (this->getOutput() == false)
            {
                this->setOutput(true);
            }
        }
    }
}
