#include <ESPAsyncWebSrv.h>
#include "wifi_secrets.h"

AsyncWebServer server(80);

const char index_html[] PROGMEM =
#include "index.h"  // unfortunatelly this only works when manipulating the html to a raw-string, and changing the file extensiomn
  ;

const char* PARAM_INPUT_1 = "name";
const char* PARAM_INPUT_2 = "state";
const char* PARAM_INPUT_3 = "value";

class PWMSwitch {
public:
  PWMSwitch(String pwm_name, int pwm_pin, int potentiometer_pin) {
    this->name = pwm_name;
    this->pwm_pin = pwm_pin;
    this->potentiometer_pin = potentiometer_pin;
    this->output_s = false;
    this->duty_cycle = 0;
  }

  void setup() {
    this->pwm_channel = this->next_free_pwm_channel;
    ledcSetup(this->pwm_channel, this->PWM_FREQ, this->RESOLUTION_BITS);
    ledcAttachPin(this->pwm_pin, pwm_channel);
    this->next_free_pwm_channel++;
  }

  String getName() {
    return this->name;
  }

  void setOutput(bool on) {
    if (on) {
      ledcWrite(this->pwm_channel, this->duty_cycle);
    } else {
      ledcWrite(this->pwm_channel, 0);
    }
    this->output_s = on;
  }

  bool getOutput() {
    return this->output_s;
  }

  void setValue(int percentage) {
    this->duty_cycle = ((float)this->MAX_VALUE * (float)percentage) / 100;
    if (this->duty_cycle == 0) {  // Turn off to avoid leak current
      this->setOutput(false);
    }
    else if (this->getOutput()) {
      ledcWrite(this->pwm_channel, this->duty_cycle);
    }
  }

  PWMSwitch* address(void) {
    return this;
  }

  int getValue() {
    return (this->duty_cycle * 100) / this->MAX_VALUE;
  }

  static int next_free_pwm_channel;  // TODO protected
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
};
int PWMSwitch::next_free_pwm_channel = 0;

static std::vector<PWMSwitch*> switches = {
  //         Name                 PIn, PotiPin
  new PWMSwitch("Ruecklicht",     13,  15),
  new PWMSwitch("Rueckfahrlicht", 27,  17),
  new PWMSwitch("BremsLicht",     33,  21),
  new PWMSwitch("Blinker",        32,  23),
};

String processor(const String& var) {
  String ret_str = String();
  if (var == "BUTTON_REPLACE") {
    for (auto pwm_switch : switches) {
      ret_str += "<tr>";
      ret_str += "<td>" + pwm_switch->getName() + "</td>";

      // Checkbox
      ret_str += "<td><input type=\"checkbox\" id=\"" + pwm_switch->getName() + "\"";
      if (pwm_switch->getOutput()) {
        ret_str += " checked ";
      }
      ret_str += "></td>";

      // Slider
      ret_str += "<td><input type=\"range\" min=\"1\" max=\"100\" value=\"";
      ret_str += String(pwm_switch->getValue());
      ret_str += "\" class=\"slider\" id=\"" + pwm_switch->getName() + "\"></td>";
      ret_str += "</tr>";
    }
  }
  return ret_str;
}

PWMSwitch* getSwitchByName(String name) {
  for (auto pwm_switch : switches) {
    if (pwm_switch->getName() == name) {      
      Serial.print("Found switch: ");
      Serial.println(pwm_switch->getName());
      return pwm_switch;
    }
  }
  throw std::invalid_argument("Didn't find PWMSwitch");
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  WiFi.setHostname("headlight1");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  for (auto pwm_switch : switches) {
    Serial.print("Setting up switch: ");
    pwm_switch->setup();
  }
  Serial.println("Setup switches");

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/update", HTTP_GET, [](AsyncWebServerRequest* request) {
    String inputMessage1;
    String inputMessage2;
    // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
    try {
      if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2)) {
        inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
        inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
        Serial.print(inputMessage1);
        Serial.print(" needs output: ");
        Serial.println(inputMessage2);

        getSwitchByName(inputMessage1)->setOutput(inputMessage2.toInt());
      } else if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_3)) {
        inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
        inputMessage2 = request->getParam(PARAM_INPUT_3)->value();
        Serial.print(inputMessage1);
        Serial.print(" needs value: ");
        Serial.println(inputMessage2);

        getSwitchByName(inputMessage1)->setValue(inputMessage2.toInt());
      } else {
        inputMessage1 = "No message sent";
        inputMessage2 = "No message sent";
        request->send(400, "text/plain", "Not OK");
        return;
      }
    } catch (std::invalid_argument& ex) {
      Serial.print("Error: ");
      Serial.println(ex.what());
    }
    request->send(200, "text/plain", "OK");
  });

  server.onNotFound([](AsyncWebServerRequest* request) {
    String with_slash = request->url();
    Serial.print("Weird request: ");
    Serial.println(with_slash);
  });
  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
}
