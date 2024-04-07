#ifndef PWM_SWITCH_H
#define PWM_SWITCH_H

#include <WString.h>
#include <stdint.h>
#include <esp32-hal.h>

/**
 * @brief Class for a PWM-based Switch (Light)
 */
class PWMSwitch
{
public:
    /**
     * @brief Construct a new PWMSwitch object
     *
     * @param pwm_name Name of switch
     * @param pwm_pin Pin to toggle PWM
     * @param potentiometer_pin Pin of potentiometer
     */
    PWMSwitch(const String &pwm_name, uint8_t pwm_pin, uint8_t potentiometer_pin)
        : m_name(pwm_name),
          m_pwm_pin(pwm_pin),
          m_potentiometer_pin(potentiometer_pin) {}

    /**
     * @brief Destroy the PWMSwitch object
     */
    virtual ~PWMSwitch() = default;

    /**
     * @brief Return next pwm channel. Only call once per instance!
     *
     * @return next pwm channel
     */
    static uint8_t getNextPWMChannel();

    /**
     * @brief Sets up PWM. Attach timer, set pin and frequencies
     */
    virtual void setup();

    /**
     * @brief Returns name of switch
     *
     * @return Name
     */
    const String &getName() const;

    /**
     * @brief Enables PWM. When switching on, set m_duty_cycle to previous set value
     *
     * @param on Boolean to set on (true) or off (false)
     */
    virtual void setOutput(bool on);

    /**
     * @brief Returns if the PWM Switch is powered on
     *
     * @return true if on, false if off
     */
    virtual bool getOutput() const;

    /**
     * @brief Sets duty-cycle. (calculated from given percentage)
     * If given percentage is 0, switch off
     * If given percentage above 0, switch (on) to given value
     *
     * @param percentage Percentage (0-100) to set value
     */
    virtual void setValue(uint8_t percentage);

    /**
     * @brief Returns current duty-cycle in percent
     *
     * @return duty cycle
     */
    uint8_t getValue() const;

    /**
     * @brief Measures potentiometer pin. If value changed over threshold (CHANGE_PERC), set value to pwm
     */
    virtual void measure_potentiometer_set_value();

private:
    static uint8_t m_next_free_pwm_channel;            // Internal counter. For each switch a counter
    const String m_name;                               // Name for logging
    uint8_t m_duty_cycle = 0;                          // Currently set duty-cycle
    const uint8_t m_pwm_pin;                           // PWM-Pin
    const uint8_t m_pwm_channel = getNextPWMChannel(); // PWM channel used for PWM pin

    // PWM-Constants
    static const uint16_t PWM_FREQ = 500;     // Frequency for pwm
    static const uint8_t RESOLUTION_BITS = 8; // Could get up to 16 bit, but we don't need it here
protected:
    bool m_output_enable = false;                  // output enable
    const uint8_t m_potentiometer_pin;             // Analog pin for potentiometer
    uint8_t m_last_measure_changed_value_perc = 0; // Last measured potentiometer value (to detect changes)

    //  Analog-Constants
    static const uint16_t MAX_VALUE = (1 << RESOLUTION_BITS) - 1;
    static const uint16_t MAX_ANALOG_IN = 0xFFF; // Maximum analog value (12 bit)
    static const uint8_t CHANGE_PERC = 5;        // Avoid oscillation, only trigger if this value differ
};

#endif // PWM_SWITCH_H
