#ifndef PWM_SWITCH_H
#define PWM_SWITCH_H

class PWMSwitch {
public:
  PWMSwitch(String pwm_name, int pwm_pin, int potentiometer_pin)
    : name(pwm_name),
      pwm_pin(pwm_pin),
      pwm_channel(getNextPWMChannel()),
      potentiometer_pin(potentiometer_pin),
      output_s(false),
      duty_cycle(0),
      last_measure_changed_value_perc(-1) {}

  // Return next pwm channel. Only call once per instance!
  static uint getNextPWMChannel() {
    return next_free_pwm_channel++;
  }
  // Sets up PWM
  virtual void setup() {
    ledcSetup(this->pwm_channel, this->PWM_FREQ, this->RESOLUTION_BITS);
    ledcAttachPin(this->pwm_pin, pwm_channel);
  }

  // Returns name of switch
  String getName() {
    return this->name;
  }

  // Enables PWM. When switching on, set duty_cycle to previous set value
  virtual void setOutput(bool on) {
    if (on && this->duty_cycle != 0) {
      ledcWrite(this->pwm_channel, this->duty_cycle);
    } else {
      ledcWrite(this->pwm_channel, 0);
    }
    this->output_s = on;
  }

  // Returns if the PWM Switch is powered on
  bool getOutput() {
    return this->output_s;
  }

  // Sets duty-cycle. (calculated from given percentage)
  // If given percentage is 0, switch off
  // If given percentage above 0, switch (on) to given value
  virtual void setValue(uint percentage) {
    this->duty_cycle = ((float)this->MAX_VALUE * (float)percentage) / 100;
    if (this->duty_cycle == 0) {  // Turn off to avoid leak current
      this->setOutput(false);
    } else if (this->getOutput()) {
      ledcWrite(this->pwm_channel, this->duty_cycle);
    }
  }

  // Returns current duty-cycle in percent
  int getValue() {
    return (this->duty_cycle * 100) / this->MAX_VALUE;
  }

  // Measures potentiometer pin. If value changed over threshold (CHANGE_PERC), set value to pwm
  virtual void measure_potentiometer_set_value() {
    int sensor_value = analogRead(this->potentiometer_pin);
    sensor_value = (100 * sensor_value) / this->MAX_ANALOG_IN;

    uint diff = 0;
    if (sensor_value > this->last_measure_changed_value_perc) {
      diff = sensor_value - this->last_measure_changed_value_perc;
    } else {
      diff = this->last_measure_changed_value_perc - sensor_value;
    }
    if (diff > this->CHANGE_PERC) {
      this->setValue(sensor_value);
      this->last_measure_changed_value_perc = sensor_value;
      // If value is below CHANGE_PERC turn it off
      if (sensor_value < this->CHANGE_PERC) {
        if (this->getOutput() == true) {
          this->setOutput(false);
        }
      } else {
        if (this->getOutput() == false) {
          this->setOutput(true);
        }
      }
    }
  }

protected:
  String name;                           // Name for logging
  bool output_s;                         // output enable
  uint duty_cycle;                       // Currently set duty-cycle
  const uint pwm_pin;                    // PWM-Pin
  const uint pwm_channel;                // PWM channel used for PWM pin
  const uint potentiometer_pin;          // Anlog pin for potentiometer
  uint last_measure_changed_value_perc;  // Last measued potentiometer value (to detect changes)

  // Constants
  //   PWM
  static const int PWM_FREQ = 500;
  static const int RESOLUTION_BITS = 8;
  //   Analog
  static const int MAX_VALUE = (1 << RESOLUTION_BITS) - 1;
  static const int MAX_ANALOG_IN = 0xFFF;  // Maximum analog value (12 bit)
  static const int CHANGE_PERC = 5;        // Avoid oscillation, only trigger if this value differ
private:
  static int next_free_pwm_channel;  // Internal counter. For each switch a counter
};
int PWMSwitch::next_free_pwm_channel = 0;

#endif  // PWM_SWITCH_H
