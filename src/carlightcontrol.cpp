#include <ESPAsyncWebServer.h>
#include <Update.h>

#include "wifi_secrets.h"
#include "pwm_switch.h"
#include "blinking_pwm_switch.h"

#define ENABLE_POTENTIOMETER /* You might want to disable them for testing */

AsyncWebServer server(80);

// This is kind of a hack to store file content into a variable. In this case the index.h(tml)
const char index_html[] PROGMEM =
#include "index.h" // unfortunatelly this only works when manipulating the html to a raw-string, and changing the file extension
    ;

// clang-format off
// Create instances of Lights
//                         Name             PWMPin, PotiPin
PWMSwitch ruecklicht{     "Ruecklicht",     13,     35};
PWMSwitch bremslicht{     "BremsLicht",     27,     34};
BlinkingPWMSwitch blinker{"Blinker",        33,     39};
PWMSwitch rueckfahrlicht{ "Rueckfahrlicht", 32,     36};
// clang-format on

// Add lights to vector
const static std::vector<PWMSwitch *> switches = {
    &ruecklicht,
    &bremslicht,
    &blinker,
    &rueckfahrlicht};

// Callback for HTTP-Request. Manipulates index.h(tml) and replaces each BUTTON_REPLACE with switch content
String processor(const String &var)
{
    auto ret_str = String();
    if (var == "BUTTON_REPLACE")
    {
        for (auto pwm_switch : switches)
        {
            ret_str += "<tr>";
            ret_str += "<td>" + pwm_switch->getName() + "</td>";

            // Checkbox
            ret_str += R"(<td><input type="checkbox" id=")" + pwm_switch->getName() + "\"";
            if (pwm_switch->getOutput())
            {
                ret_str += " checked ";
            }
            ret_str += "></td>";

            // Slider
            ret_str += R"(<td><input type="range" min="1" max="100" value=")";
            ret_str += String(pwm_switch->getValue());
            ret_str += R"(" class="slider" id=")" + pwm_switch->getName() + "\"></td>";
            ret_str += "</tr>";
        }
    }
    else if (var == "DATE")
    {
        ret_str = __DATE__ " " __TIME__;
    }
    return ret_str;
}

// Returns switch by name or raises Exception if not found
PWMSwitch *getSwitchByName(const String &name)
{
    for (auto pwm_switch : switches)
    {
        if (pwm_switch->getName() == name)
        {
            return pwm_switch;
        }
    }
    throw std::invalid_argument("Didn't find PWMSwitch");
}

const char *PARAM_INPUT_1 = "name";
const char *PARAM_INPUT_2 = "state";
const char *PARAM_INPUT_3 = "value";

// Setup routine. Setups Serial, WiFi, Switches and instantiates callback for HTTP_Server
void setup()
{
    Serial.begin(115200);

    WiFi.begin(ssid, password);
    WiFiClass::setHostname("headlight1");
    while (WiFiClass::status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    for (auto pwm_switch : switches)
    {
        Serial.print("Setting up switch: ");
        pwm_switch->setup();
    }
    Serial.println("Setup switches");

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/html", index_html, processor); });

    server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        String inputMessage1;
        String inputMessage2;
        // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
        try {
            // Toggle light on/off
            if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2)) {
                inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
                inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
                Serial.print(inputMessage1);
                Serial.print(" needs output: ");
                Serial.println(inputMessage2);
                getSwitchByName(inputMessage1)->setOutput(inputMessage2.toInt());
            }
            // Set PWM-value of light
            else if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_3)) {
                inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
                inputMessage2 = request->getParam(PARAM_INPUT_3)->value();
                Serial.print(inputMessage1);
                Serial.print(" needs value: ");
                Serial.println(inputMessage2);
                getSwitchByName(inputMessage1)->setValue(inputMessage2.toInt());
            } 
            // Unknown
            else {
                inputMessage1 = "No message sent";
                inputMessage2 = "No message sent";
                request->send(400, "text/plain", "Not OK");
                return;
            }
        } 
        catch (std::invalid_argument& ex) {
            Serial.print("Error: ");
            Serial.println(ex.what());
        }
        request->send(200, "text/plain", "OK"); });

    // Handle firmware update  (OTA)
    server.on(
        "/upload", HTTP_POST, [&](AsyncWebServerRequest *request)
        {
        // the request handler is triggered after the upload has finished... 
        // create the response, add header, and send response
        AsyncWebServerResponse *response = request->beginResponse((Update.hasError())?500:200, "text/plain", (Update.hasError())?"FAIL":"OK");
        response->addHeader("Connection", "close");
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
        delay(200);
        ESP.restart(); },
        [&](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
        {
            // Upload handler chunks in data
            if (!index)
            {
                Serial.println("Starting firmware update");
                // Switch off all lamps
                for (auto pwm_switch : switches)
                {
                    pwm_switch->setValue(0);
                }

                int cmd = (filename == "filesystem") ? U_SPIFFS : U_FLASH;
                if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd))
                { // Start with max available size
                    Update.printError(Serial);
                    return request->send(400, "text/plain", "OTA could not begin");
                }
            }

            // Write chunked data to the free sketch space
            if (len)
            {
                if (Update.write(data, len) != len)
                {
                    return request->send(400, "text/plain", "OTA could not begin");
                }
            }

            // if the final flag is set then this is the last frame of data
            if (final)
            {
                Serial.println("Finished update");
                if (!Update.end(true))
                { // true to set the size to the current progress
                    Update.printError(Serial);
                    return request->send(400, "text/plain", "Could not end OTA");
                }
            }
            else
            {
                return;
            }
        });

    // Set callback if request is unknown
    server.onNotFound([](AsyncWebServerRequest const *request)
                      {
        String unkown_request = request->url();
        Serial.print("Unknown request: ");
        Serial.println(unkown_request); });

    // Start server
    server.begin();
}

// Loop which polls potentiometer values. If a value changed, override current one (else keep the one either set by previous pwm or from server)
void loop()
{
#ifdef ENABLE_POTENTIOMETER
    for (auto pwm_switch : switches)
    {
        pwm_switch->measure_potentiometer_set_value();
    }
#endif
    delay(100);
}
