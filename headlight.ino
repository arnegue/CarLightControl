#include <ESPAsyncWebSrv.h>
#include "wifi.h"

#define AMOUNT_CHARS_NEW 128

char esp_buffer[AMOUNT_CHARS_NEW];
AsyncWebServer server(80);

const char index_html[] PROGMEM =
#include "index.h" // unfortunatelly this only works when manipulating the html to a raw-string, and changing the file extensiomn
  ;

const char* PARAM_INPUT_1 = "name";
const char* PARAM_INPUT_2 = "state";
const char* PARAM_INPUT_3 = "value";

class PWMSwitch {
  public :
    PWMSwitch(String pwm_name, int pwm_pin, int potentiometer_pin) {
      this->name = pwm_name;
      this->pwm_pin = pwm_pin;
      this->potentiometer_pin = potentiometer_pin;
    }
    String getName() {
      return this->name;
    }
    void setValue(int percentage){
      // TODO calculation
    }
  protected:
    String name;
    int pwm_pin;
    int potentiometer_pin;
};

std::vector<PWMSwitch> switches = {
  PWMSwitch("Ruecklicht", 0, 0),
  PWMSwitch("BremsLicht", 0, 0),
  PWMSwitch("Blinker", 0, 0),
  PWMSwitch("Rueckfahrlicht", 0, 0),
};

String processor(const String& var)
{
  if (var == "BUTTON_REPLACE"){
    String ret_str = String();
    for (auto pwm_switch : switches) {
      ret_str += "<tr>";
      ret_str += "<td>" + pwm_switch.getName() + "</td>";
      ret_str += "<td><input type=\"checkbox\" id=\"" + pwm_switch.getName() + "\"></td>";
      ret_str += "<td><input type=\"range\" min=\"1\" max=\"100\" value=\"50\" class=\"slider\" id=\"" + pwm_switch.getName() +"\"></td>";
      ret_str += "</tr>";
    }
    return ret_str;
  }
  return String();
}

void setup() {
  Serial.begin(115200);

  Serial.print("Testcontent:");
  Serial.println(index_html);

  WiFi.begin(ssid, password);
  WiFi.setHostname("headlight1");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  //server.serveStatic("/index.html", SPIFFS, "/index.html");

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest * request) {
    String inputMessage1;
    String inputMessage2;
    // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
    if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2)) {
      inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      Serial.print(inputMessage1);
      Serial.print(" needs output: ");
      Serial.println(inputMessage2);
    }
    else if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_3)) {
      inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
      inputMessage2 = request->getParam(PARAM_INPUT_3)->value();
      Serial.print(inputMessage1);
      Serial.print(" needs value: ");
      Serial.println(inputMessage2);
    }
    else {
      inputMessage1 = "No message sent";
      inputMessage2 = "No message sent";
      Serial.println("else bad");
    }
    request->send(200, "text/plain", "OK");
  });

  server.onNotFound([](AsyncWebServerRequest * request) {
    String with_slash = request->url();
    Serial.print("Weird request: ");
    Serial.println(with_slash);
  });
  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:

}
