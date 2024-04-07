#include "switch_web_server.hpp"
#include <Update.h>

// This is kind of a hack to store file content into a variable. In this case the index.h(tml)
const char index_html[] PROGMEM =
#include "index.h" // unfortunatelly this only works when manipulating the html to a raw-string, and changing the file extension
    ;

void SwitchWebServer::handle_default(AsyncWebServerRequest *request)
{
    request->send_P(200, "text/html", index_html, processor);
}

void SwitchWebServer::handle_update(AsyncWebServerRequest *request)
{
    String inputMessage1;
    String inputMessage2;
    // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
    try
    {
        // Toggle light on/off
        if (request->hasParam(PARAM_NAME) && request->hasParam(PARAM_STATE))
        {
            inputMessage1 = request->getParam(PARAM_NAME)->value();
            inputMessage2 = request->getParam(PARAM_STATE)->value();
            Serial.print(inputMessage1);
            Serial.print(" needs output: ");
            Serial.println(inputMessage2);
            getSwitchByName(inputMessage1)->setOutput(inputMessage2.toInt());
        }
        // Set PWM-value of light
        else if (request->hasParam(PARAM_NAME) && request->hasParam(PARAM_VALUE))
        {
            inputMessage1 = request->getParam(PARAM_NAME)->value();
            inputMessage2 = request->getParam(PARAM_VALUE)->value();
            Serial.print(inputMessage1);
            Serial.print(" needs value: ");
            Serial.println(inputMessage2);
            getSwitchByName(inputMessage1)->setValue(static_cast<uint8_t>(inputMessage2.toInt()));
        }
        // Unknown
        else
        {
            request->send(400, "text/plain", "Not OK");
            return;
        }
    }
    catch (std::invalid_argument &ex)
    {
        Serial.print("Error: ");
        Serial.println(ex.what());
    }
    request->send(200, "text/plain", "OK");
}

void SwitchWebServer::handle_not_found(AsyncWebServerRequest *request)
{
    String unkown_request = request->url();
    Serial.print("Unknown request: ");
    Serial.println(unkown_request);
}

void SwitchWebServer::handle_on_successful_upload(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response = request->beginResponse((Update.hasError()) ? 500 : 200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    response->addHeader("Connection", "close");
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
    delay(200);
    ESP.restart();
}

void SwitchWebServer::handle_on_upload(AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)
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

        uint8_t cmd = (filename == "filesystem") ? U_SPIFFS : U_FLASH;
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
}

PWMSwitch *SwitchWebServer::getSwitchByName(const String &name)
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

String SwitchWebServer::RequestProcessor(const String &var)
{
    auto ret_str = String();

    if (var == "BUTTON_REPLACE")
    {
        Serial.println("BUTTON_REPLACE");
        for (auto pwm_switch : switches)
        {
            Serial.println(pwm_switch->getName());
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
            ret_str += R"(<td><input style="width:100%%" type="range" min="1" max="100" value=")";
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
