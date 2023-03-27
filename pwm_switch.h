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
protected:
  String name;
  bool output_s;
  int duty_cycle;
  int pwm_pin;
  int pwm_channel;
  int potentiometer_pin;
  const int PWM_FREQ = 5000;
  const int RESOLUTION_BITS = 8;
  const int MAX_VALUE = (1 << RESOLUTION_BITS) - 1;

private:
  static int next_free_pwm_channel;
};
int PWMSwitch::next_free_pwm_channel = 0;

#endif // PWM_SWITCH_H
