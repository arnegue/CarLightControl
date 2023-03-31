#ifndef BLINKING_PWM_SWITCH_H
#define BLINKING_PWM_SWITCH_H

#include <set>
#include "pwm_switch.h"

hw_timer_t *Timer0_Cfg = NULL;
void static_c_style_blink_callback();

class BlinkingPWMSwitch : public PWMSwitch {
  using PWMSwitch::PWMSwitch;  // Don't redefine constructor
public:
  void setup() {
    PWMSwitch::setup();
    Timer0_Cfg = timerBegin(0, 8000, true);
    timerAttachInterrupt(Timer0_Cfg, &static_c_style_blink_callback, true);
    timerAlarmWrite(Timer0_Cfg, 10000, true);
  }

  void setOutput(bool on) override {
    PWMSwitch::setOutput(on);
    if (on) {
      if (blinkers.size() == 0) {
        timerAlarmEnable(Timer0_Cfg);
      }
      blinkers.insert(this);
    } else {
      auto it = std::find(blinkers.begin(), blinkers.end(), this);
      if (it != blinkers.end()) {
        blinkers.erase(this);
      }
      if (blinkers.size() == 0) {
        timerAlarmDisable(Timer0_Cfg);
      }
    }
  }

  void blink() {
    bool new_value = !this->getOutput();
    PWMSwitch::setOutput(new_value);  // Call base class to avoid toggling timer
  }

  static std::set<BlinkingPWMSwitch *> getBlinkers() {
    return blinkers;
  }

  virtual void measure_potentiometer_set_value() override {
    int sensor_value = analogRead(this->potentiometer_pin);
    sensor_value = (100 * sensor_value) / this->MAX_ANALOG_IN;
    if (this->last_measure_changed_value_perc == -1) {
      // Avoid change-detection on start-up
      this->last_measure_changed_value_perc = sensor_value;
    }
    else if (abs(sensor_value - this->last_measure_changed_value_perc) > this->CHANGE_PERC) {
      this->setValue(sensor_value);
      this->last_measure_changed_value_perc = sensor_value;

      bool is_enabled = false;
      auto it = std::find(blinkers.begin(), blinkers.end(), this);
      if (it != blinkers.end()) {
        is_enabled = true;
      }

      if (sensor_value < this->CHANGE_PERC) {
        if (is_enabled) {
          this->setOutput(false);
        }
      } else {
        if (!is_enabled) {
          this->setOutput(true);
        }
      }
    }
  }

protected:
  static std::set<BlinkingPWMSwitch *> blinkers;
};

std::set<BlinkingPWMSwitch *> BlinkingPWMSwitch::blinkers = std::set<BlinkingPWMSwitch *>();
void static_c_style_blink_callback() {
  for (auto blinker : BlinkingPWMSwitch::getBlinkers()) {
    blinker->blink();
  }
}

#endif  // BLINKING_PWM_SWITCH_H
