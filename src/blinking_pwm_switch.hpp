#ifndef BLINKING_PWM_SWITCH_H
#define BLINKING_PWM_SWITCH_H

#include <set>
#include <algorithm>
#include "pwm_switch.hpp"
#include <esp32-hal-timer.h>

void blink_callback(); // Forward declaration of blink_callback

/**
 * @brief Nearly same as PWMSwitch but also able to blink
 *
 */
class BlinkingPWMSwitch : public PWMSwitch
{
    /**
     * @brief Don't redefine constructor. Just use base constructor
     */
    using PWMSwitch::PWMSwitch;

public:
    /**
     * @brief Calls base setup and initializes timer for blinking
     */
    void setup() override;

    /**
     * @brief Destroy the Blinking PWM Switch object
     */
    virtual ~BlinkingPWMSwitch() = default;

    /**
     * @brief Returns if the PWM Switch is enabled (not just currently powered on. That depends on the blink state)
     *
     * @return true if on, false if off
     */
    bool getOutput() const override;

    /**
     * @brief  Enables blinker. If so, add it to active_blinkers-list so that timer may toggle it in callback
     * @param on Boolean to set on (true) or off (false)
     */
    void setOutput(bool on) override;

    /**
     * @brief Toggles value ("blink")
     */
    void toggle();

    /**
     * @brief Returns all current active switches
     *
     * @return current set of  active blinkers
     */
    static std::set<BlinkingPWMSwitch *> getActiveBlinkers();

protected:
    static std::set<BlinkingPWMSwitch *> active_blinkers; // List of all active blinkers
    static hw_timer_t *BlinkTimerCfg;                     // Timer need for blinking
    const uint8_t TIMER_NUMBER = 0;                       // ID of timer

    // We need a 1Hz blink frequency. So we want the callback to be called every 1000ms: 1000 = TIMER_TICKS * (TIMER_PRESCALER / 80Mhz APB_CLK )
    const uint16_t TIMER_PRESCALER = 8000;
    const uint16_t TIMER_TICKS = 10000;
    const bool TIMER_COUNT_UP_OR_DOWN = true;
};

#endif // BLINKING_PWM_SWITCH_H
