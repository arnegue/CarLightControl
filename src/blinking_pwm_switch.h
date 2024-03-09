#ifndef BLINKING_PWM_SWITCH_H
#define BLINKING_PWM_SWITCH_H

#include <set>
#include <algorithm>
#include "pwm_switch.h"
#include <esp32-hal-timer.h>

void blink_callback(); // Forward declaration of blink_callback

class BlinkingPWMSwitch : public PWMSwitch
{
    using PWMSwitch::PWMSwitch; // Don't redefine constructor
public:
    void setup() override
    {
        PWMSwitch::setup();
        // Setup timer if not already set up
        if (BlinkTimerCfg == nullptr)
        {
            BlinkTimerCfg = timerBegin(TIMER_NUMBER, TIMER_PRESCALER, TIMER_COUNT_UP_OR_DOWN);
            timerAttachInterrupt(BlinkTimerCfg, &blink_callback, true);
            timerAlarmWrite(BlinkTimerCfg, TIMER_TICKS, true);
        }
    }

    // Default base destructor
    virtual ~BlinkingPWMSwitch() = default;

    // Enables blinker. If so, add it to active_blinkers-list so that timer may toggle it in callback
    void setOutput(bool on) override
    {
        PWMSwitch::setOutput(on);
        if (on)
        {
            // Activate timer when first blinker gets enabled
            if (active_blinkers.empty())
            {
                timerAlarmEnable(BlinkTimerCfg);
            }
            active_blinkers.insert(this);
        }
        else
        {
            auto it = std::find(active_blinkers.begin(), active_blinkers.end(), this);
            if (it != active_blinkers.end())
            {
                active_blinkers.erase(this);
            }
            // Disable timer when last blinker gets disabled
            if (active_blinkers.empty())
            {
                timerAlarmDisable(BlinkTimerCfg);
            }
        }
    }

    // Toggles value ("blink")
    void toggle()
    {
        bool new_value = !this->getOutput();
        PWMSwitch::setOutput(new_value); // Call base class to avoid toggling timer
    }

    // Returns all current active switches
    static std::set<BlinkingPWMSwitch *> getActiveBlinkers()
    {
        return active_blinkers;
    }

    void measure_potentiometer_set_value() override
    {
        int sensor_value = analogRead(this->potentiometer_pin);
        sensor_value = (100 * sensor_value) / BlinkingPWMSwitch::MAX_ANALOG_IN;

        uint diff = 0;
        if (sensor_value > this->last_measure_changed_value_perc)
        {
            diff = sensor_value - this->last_measure_changed_value_perc;
        }
        else
        {
            diff = this->last_measure_changed_value_perc - sensor_value;
        }
        if (this->last_measure_changed_value_perc == -1)
        {
            // Avoid change-detection on start-up
            this->last_measure_changed_value_perc = sensor_value;
        }
        else if (diff > BlinkingPWMSwitch::CHANGE_PERC)
        {
            this->setValue(sensor_value);
            this->last_measure_changed_value_perc = sensor_value;

            bool is_enabled = false;
            auto it = std::find(active_blinkers.begin(), active_blinkers.end(), this);
            if (it != active_blinkers.end())
            {
                is_enabled = true;
            }

            if (sensor_value < BlinkingPWMSwitch::CHANGE_PERC)
            {
                if (is_enabled)
                {
                    this->setOutput(false);
                }
            }
            else
            {
                if (!is_enabled)
                {
                    this->setOutput(true);
                }
            }
        }
    }

protected:
    static std::set<BlinkingPWMSwitch *> active_blinkers; // List of all active blinkers
    static hw_timer_t *BlinkTimerCfg;                     // Timer need for blinking
    const uint8_t TIMER_NUMBER = 0;                       // ID of timer
    // We need a 1Hz blink frequency. So we want the callback to be called every 1000ms: 1000 = TIMER_TICKS * (TIMER_PRESCALER / 80Mhz APB_CLK )
    const uint16_t TIMER_PRESCALER = 8000;
    const uint16_t TIMER_TICKS = 10000;
    const bool TIMER_COUNT_UP_OR_DOWN = true;
};

std::set<BlinkingPWMSwitch *> BlinkingPWMSwitch::active_blinkers = std::set<BlinkingPWMSwitch *>();
hw_timer_t *BlinkingPWMSwitch::BlinkTimerCfg = nullptr;

// Callbacks. Toggles all active blinkers
void blink_callback()
{
    for (auto blinker : BlinkingPWMSwitch::getActiveBlinkers())
    {
        blinker->toggle();
    }
}

#endif // BLINKING_PWM_SWITCH_H
