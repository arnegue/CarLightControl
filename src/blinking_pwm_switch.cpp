#include "blinking_pwm_switch.hpp"

void BlinkingPWMSwitch::setup()
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

bool BlinkingPWMSwitch::getOutput() const
{
    auto it = std::find(activeBlinkers.begin(), activeBlinkers.end(), this);
    return it != activeBlinkers.end(); // If blinker is in the list, it's enabled
}

void BlinkingPWMSwitch::setOutput(bool on)
{
    PWMSwitch::setOutput(on);
    if (on)
    {
        // Activate timer when first blinker gets enabled
        if (activeBlinkers.empty())
        {
            timerAlarmEnable(BlinkTimerCfg);
        }
        activeBlinkers.insert(this);
    }
    else
    {
        // Remove from active blinkers if it's enabled
        if (getOutput())
        {
            activeBlinkers.erase(this);
        }
        // Disable timer when last blinker gets disabled
        if (activeBlinkers.empty())
        {
            timerAlarmDisable(BlinkTimerCfg);
        }
    }
}

void BlinkingPWMSwitch::toggle()
{
    // Call base class to avoid toggling timer (don't use getOutput. That's more like "enabled")
    PWMSwitch::setOutput(!mOutputEnable);
}

std::set<BlinkingPWMSwitch *> BlinkingPWMSwitch::getActiveBlinkers()
{
    return activeBlinkers;
}

std::set<BlinkingPWMSwitch *> BlinkingPWMSwitch::activeBlinkers = std::set<BlinkingPWMSwitch *>();
hw_timer_t *BlinkingPWMSwitch::BlinkTimerCfg = nullptr;

// Callbacks. Toggles all active blinkers
void blink_callback()
{
    for (auto blinker : BlinkingPWMSwitch::getActiveBlinkers())
    {
        blinker->toggle();
    }
}
