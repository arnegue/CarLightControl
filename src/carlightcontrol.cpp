#include "wifi_secrets.h"
#include "pwm_switch.hpp"
#include "blinking_pwm_switch.hpp"
#include "switch_web_server.hpp"

#define ENABLE_POTENTIOMETER /* You might want to disable them for testing */

// clang-format off
// Create instances of Lights
//                         Name             PWMPin, PotiPin
PWMSwitch ruecklicht{     "Ruecklicht",     13,     35}; // PWM1
PWMSwitch bremslicht{     "BremsLicht",     27,     34}; // PWM2
BlinkingPWMSwitch blinker{"Blinker",        33,     39}; // PWM3
PWMSwitch rueckfahrlicht{ "Rueckfahrlicht", 32,     36}; // PWM4
// clang-format on

// Add lights to vector
const static std::vector<PWMSwitch *> switches = {
    &ruecklicht,
    &bremslicht,
    &blinker,
    &rueckfahrlicht};

SwitchWebServer web_server(switches, 80);

/**
 * @brief Setup routine. Setups Serial, WiFi, Switches and SwitchWebServer
 */
void setup()
{
    Serial.begin(115200);

    // WiFi
    WiFi.begin(ssid, password);
    WiFiClass::setHostname("headlight1");
    while (WiFiClass::status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.print("Connected to WiFi: ");
    Serial.println(ssid);

    // Switches
    for (auto pwm_switch : switches)
    {
        Serial.print("Setting up switch: ");
        Serial.println(pwm_switch->getName());
        pwm_switch->setup();
    }
    Serial.println("Setup switches");

    // Server
    Serial.println("Starting server");
    web_server.begin();
    Serial.println("Started server");
}

/**
 * @brief  Loop which polls potentiometer values. If a value changed, override current one (else keep the one either set by previous pwm or from server)
 */
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
