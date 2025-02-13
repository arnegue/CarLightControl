#include "wifi_secrets.h"
#include "pwm_switch.hpp"
#include "blinking_pwm_switch.hpp"
#include "switch_web_server.hpp"

#define ENABLE_POTENTIOMETER /* You might want to disable them for testing */

// clang-format off
// Create instances of Lights
//                         Name             PWMPin, PotiPin
PWMSwitch ruecklicht{     "Ruecklicht",     13,     35}; // PWM1
PWMSwitch bremslicht{     "BremsLicht 1",   27,     34}; // PWM2
BlinkingPWMSwitch blinker{"Blinker",        33,     39}; // PWM3
PWMSwitch rueckfahrlicht{ "Bremslicht 2",   32,     36}; // PWM4
// clang-format on

// Add lights to vector
const static std::vector<PWMSwitch *> switches = {
    &ruecklicht,
    &bremslicht,
    &rueckfahrlicht,
    &blinker};

SwitchWebServer web_server(switches, 80);

void CheckWiFiStatus(wl_status_t &previousState)
{
    auto wifiState = WiFiClass::status();
    if (wifiState != WL_CONNECTED)
    {
        Serial.printf("Connecting to WiFi. Current state: %d\n", wifiState);
    }
    else if (wifiState == WL_CONNECTED && previousState != wifiState)
    {
        Serial.printf("Connected to WiFi \"%s\" with IP: ", ssid);
        Serial.println(WiFi.localIP());
        previousState = wifiState;
    }
}

wl_status_t lastWiFiState = WL_DISCONNECTED;

/**
 * @brief Setup routine. Setups Serial, WiFi, Switches and SwitchWebServer
 */
void setup()
{
    Serial.begin(115200);

    // WiFi
    WiFi.begin(ssid, password);
    WiFiClass::setHostname("headlight1");
    CheckWiFiStatus(lastWiFiState);

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
 * @brief  Loop which polls wifi state and potentiometer values. If a value changed, override current one (else keep the one either set by previous pwm or from server)
 */
void loop()
{
    CheckWiFiStatus(lastWiFiState);
#ifdef ENABLE_POTENTIOMETER
    for (auto pwm_switch : switches)
    {
        pwm_switch->measurePotentiometerSetValue();
    }
#endif
    delay(100);
}
