#ifndef PWM_SWITCH_H
#define PWM_SWITCH_H

class PWMSwitch {
public:
  PWMSwitch(String pwm_name, int pwm_pin, int potentiometer_pin) {
    this->name = pwm_name;
    this->pwm_pin = pwm_pin;
    this->potentiometer_pin = potentiometer_pin;
    this->output_s = false;
    this->duty_cycle = 0;
    this->last_measure_changed_value_perc = 0;
  }

  virtual void setup() {
    this->pwm_channel = this->next_free_pwm_channel;
    ledcSetup(this->pwm_channel, this->PWM_FREQ, this->RESOLUTION_BITS);
    ledcAttachPin(this->pwm_pin, pwm_channel);
    this->next_free_pwm_channel++;
  }

  String getName() {
    return this->name;
  }

  virtual void setOutput(bool on) {
    if (on && this->duty_cycle != 0) {
      ledcWrite(this->pwm_channel, this->duty_cycle);
    } else {
      ledcWrite(this->pwm_channel, 0);
    }
    this->output_s = on;
  }

  bool getOutput() {
    return this->output_s;
  }

  virtual void setValue(int percentage) {
    this->duty_cycle = ((float)this->MAX_VALUE * (float)percentage) / 100;
    if (this->duty_cycle == 0) {  // Turn off to avoid leak current
      this->setOutput(false);
    }
    else if (this->getOutput()) {
      ledcWrite(this->pwm_channel, this->duty_cycle);
    }
  }

  int getValue() {
    return (this->duty_cycle * 100) / this->MAX_VALUE;
  }
  
  virtual void measure_potentiometer_set_value() {
    int sensor_value = analogRead(this->potentiometer_pin);
    sensor_value = (100 * sensor_value) / this->MAX_ANALOG_IN;
    if (abs(sensor_value - this->last_measure_changed_value_perc) > this->CHANGE_PERC) {
      this->setValue(sensor_value);
      this->last_measure_changed_value_perc = sensor_value;
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
  String name;  // Name for logging and showing in web-ui
  bool output_s;  // output enable
  int duty_cycle;
  int pwm_pin;
  int pwm_channel;
  int potentiometer_pin;
  int last_measure_changed_value_perc;

  // Constants
  //   PWM
  static const int PWM_FREQ = 5000;
  static const int RESOLUTION_BITS = 8;
  //   Analog
  static const int MAX_VALUE = (1 << RESOLUTION_BITS) - 1;
  static const int MAX_ANALOG_IN = 0xFFF;  // Maximum analaog value (12 bit)
  static const int CHANGE_PERC = 5;        // Avoid oscillation, only trigger if this value differ
private:
  static int next_free_pwm_channel;
};
int PWMSwitch::next_free_pwm_channel = 0;

#endif // PWM_SWITCH_H
