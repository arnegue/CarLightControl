#ifndef PWM_SWITCH_H
#define PWM_SWITCH_H

#include <WString.h>
#include <stdint.h>
#include <esp32-hal.h>

class PWMSwitch
{
public:
    PWMSwitch(const String &pwm_name, uint8_t pwm_pin, uint8_t potentiometer_pin)
        : name(pwm_name),
          pwm_pin(pwm_pin),
          pwm_channel(getNextPWMChannel()),
          potentiometer_pin(potentiometer_pin) {}

    // Default base destructor
    virtual ~PWMSwitch() = default;

    // Return next pwm channel. Only call once per instance!
    static uint8_t getNextPWMChannel()
    {
        return next_free_pwm_channel++;
    }
    // Sets up PWM
    virtual void setup()
    {
        ledcSetup(this->pwm_channel, PWMSwitch::PWM_FREQ, PWMSwitch::RESOLUTION_BITS);
        ledcAttachPin(this->pwm_pin, pwm_channel);
    }

    // Returns name of switch
    const String& getName() const
    {
        return this->name;
    }

    // Enables PWM. When switching on, set duty_cycle to previous set value
    virtual void setOutput(bool on)
    {
        if (on && this->duty_cycle != 0)
        {
            ledcWrite(this->pwm_channel, this->duty_cycle);
        }
        else
        {
            ledcWrite(this->pwm_channel, 0);
        }
        this->output_s = on;
    }

    // Returns if the PWM Switch is powered on
    bool getOutput() const
    {
        return this->output_s;
    }

    // Sets duty-cycle. (calculated from given percentage)
    // If given percentage is 0, switch off
    // If given percentage above 0, switch (on) to given value
    virtual void setValue(uint8_t percentage)
    {
        this->duty_cycle = (PWMSwitch::MAX_VALUE * percentage) / 100;
        if (this->duty_cycle == 0)
        { // Turn off to avoid leak current
            this->setOutput(false);
        }
        else if (this->getOutput())
        {
            ledcWrite(this->pwm_channel, this->duty_cycle);
        }
    }

    // Returns current duty-cycle in percent
    int getValue() const
    {
        return (this->duty_cycle * 100) / PWMSwitch::MAX_VALUE;
    }

    // Measures potentiometer pin. If value changed over threshold (CHANGE_PERC), set value to pwm
    virtual void measure_potentiometer_set_value()
    {
        uint16_t sensor_value = analogRead(this->potentiometer_pin);
        sensor_value = (100 * sensor_value) / PWMSwitch::MAX_ANALOG_IN;

        uint diff = 0;
        if (sensor_value > this->last_measure_changed_value_perc)
        {
            diff = sensor_value - this->last_measure_changed_value_perc;
        }
        else
        {
            diff = this->last_measure_changed_value_perc - sensor_value;
        }
        if (diff > PWMSwitch::CHANGE_PERC)
        {
            this->setValue(sensor_value); // TODO uint16_t
            this->last_measure_changed_value_perc = sensor_value;
            // If value is below CHANGE_PERC turn it off
            if (sensor_value < PWMSwitch::CHANGE_PERC)
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

protected:
    const String name;                           // Name for logging
    bool output_s = false;                       // output enable
    uint8_t duty_cycle = 0;                      // Currently set duty-cycle
    const uint8_t pwm_pin;                       // PWM-Pin
    const uint8_t pwm_channel;                   // PWM channel used for PWM pin
    const uint8_t potentiometer_pin;             // Analog pin for potentiometer
    uint8_t last_measure_changed_value_perc = 0; // Last measured potentiometer value (to detect changes)

    // Constants
    //   PWM
    static const int PWM_FREQ = 500;
    static const int RESOLUTION_BITS = 8;
    //   Analog
    static const int MAX_VALUE = (1 << RESOLUTION_BITS) - 1;
    static const int MAX_ANALOG_IN = 0xFFF; // Maximum analog value (12 bit)
    static const int CHANGE_PERC = 5;       // Avoid oscillation, only trigger if this value differ
private:
    static uint8_t next_free_pwm_channel; // Internal counter. For each switch a counter
};

#endif // PWM_SWITCH_H
