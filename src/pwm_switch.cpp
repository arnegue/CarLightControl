#include "pwm_switch.hpp"

uint8_t PWMSwitch::m_next_free_pwm_channel = 0;

uint8_t PWMSwitch::getNextPWMChannel()
{
    return m_next_free_pwm_channel++;
}

void PWMSwitch::setup()
{
    ledcSetup(this->m_pwm_channel, PWMSwitch::PWM_FREQ, PWMSwitch::RESOLUTION_BITS);
    ledcAttachPin(this->m_pwm_pin, m_pwm_channel);
}

const String &PWMSwitch::getName() const
{
    return this->m_name;
}

void PWMSwitch::setOutput(bool on)
{
    if (on && this->m_duty_cycle != 0)
    {
        ledcWrite(this->m_pwm_channel, this->m_duty_cycle);
    }
    else
    {
        ledcWrite(this->m_pwm_channel, 0);
    }
    this->m_output_enable = on;
}

bool PWMSwitch::getOutput() const
{
    return this->m_output_enable;
}

void PWMSwitch::setValue(uint8_t percentage)
{
    if (percentage > 100)
    {
        percentage = 100;
    }
    this->m_duty_cycle = (PWMSwitch::MAX_VALUE * percentage) / 100;
    if (this->m_duty_cycle == 0)
    { // Turn off to avoid leak current
        this->setOutput(false);
    }
    else if (this->getOutput())
    {
        ledcWrite(this->m_pwm_channel, this->m_duty_cycle);
    }
}

uint8_t PWMSwitch::getValue() const
{
    return (this->m_duty_cycle * 100) / PWMSwitch::MAX_VALUE;
}

void PWMSwitch::measure_potentiometer_set_value()
{
    uint16_t sensor_value = analogRead(this->m_potentiometer_pin);

    uint8_t pwm_value = (100 * sensor_value) / PWMSwitch::MAX_ANALOG_IN;

    uint16_t diff = 0;
    if (pwm_value > this->m_last_measure_changed_value_perc)
    {
        diff = pwm_value - this->m_last_measure_changed_value_perc;
    }
    else
    {
        diff = this->m_last_measure_changed_value_perc - pwm_value;
    }
    if (diff > PWMSwitch::CHANGE_PERC)
    {
        this->setValue(pwm_value);
        this->m_last_measure_changed_value_perc = pwm_value;
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
