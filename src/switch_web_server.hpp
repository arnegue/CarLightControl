#ifndef SWITCH_WEB_SERVER_H
#define SWITCH_WEB_SERVER_H

#include <vector>
#include <WString.h>
#include <iostream>
#include <functional>
#include <ESPAsyncWebServer.h>
#include <memory>

#include "pwm_switch.hpp"

// Sonar-lints wrapper if compiler doesn't support make_unique yet
template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args &&...args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

/**
 * @brief WebServer for given switches
 */
class SwitchWebServer
{
public:
    /**
     * @brief Construct a new Switch Web Server. Sets callbacks (via std::bind to WebServer's callbacks)
     *
     * @param switches Vector of switches
     * @param port HTTP-Ports
     */
    SwitchWebServer(const std::vector<PWMSwitch *> &switches, const uint16_t port) : switches(switches), server(make_unique<AsyncWebServer>(port))
    {
        ArRequestHandlerFunction default_function = std::bind(&SwitchWebServer::handle_default, this, std::placeholders::_1);
        server->on("/", HTTP_GET, default_function);

        ArRequestHandlerFunction update_function = std::bind(&SwitchWebServer::handle_update, this, std::placeholders::_1);
        server->on("/update", update_function);

        ArRequestHandlerFunction on_successful_upload_function = std::bind(&SwitchWebServer::handle_on_successful_upload, this, std::placeholders::_1);
        ArUploadHandlerFunction on_upload_function = std::bind(&SwitchWebServer::handle_on_upload, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6);
        server->on("/upload", HTTP_POST, on_successful_upload_function, on_upload_function);

        ArRequestHandlerFunction on_not_found_function = std::bind(&SwitchWebServer::handle_not_found, this, std::placeholders::_1);
        server->onNotFound(on_not_found_function);

        processor = std::bind(&SwitchWebServer::RequestProcessor, this, std::placeholders::_1);
    }

    /**
     * @brief Starts server
     */
    void begin()
    {
        server->begin();
    }

private:
    /**
     * @brief Handles default request (shows page)
     *
     * @param request request object to handle
     */
    void handle_default(AsyncWebServerRequest *request);

    /**
     * @brief Handles Update-calls (toggle switch; set value)
     *
     * @param request request object to handle
     */
    void handle_update(AsyncWebServerRequest *request);

    /**
     * @brief Handles unknown requests
     *
     * @param request request object to handle
     */
    void handle_not_found(AsyncWebServerRequest *request);

    /**
     * @brief Handles requests after firmware was successfully uploaded
     *
     * @param request request object to handle
     */
    void handle_on_successful_upload(AsyncWebServerRequest *request);

    /**
     * @brief Handles upload-request function
     *
     * @param request request object to handle
     * @param filename filename to upload
     * @param index index to write
     * @param data pointer to data chunk
     * @param len length of chunk to write
     */
    void handle_on_upload(AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final);

    /**
     * @brief Returns switch by name or raises Exception if not found
     *
     * @param name Name of switch
     * @return Returns Pointer of switch if found
     */
    PWMSwitch *getSwitchByName(const String &name);

    /**
     * @brief Manipulates HTML file of given string to replace
     *
     * @param var String to replace (BUTTON_REPLACE and DATE)
     * @return String of whole HTMl-document with replaced string
     */
    String RequestProcessor(const String &var);

    const String PARAM_NAME = "name";   // GET-Parameter "Name"
    const String PARAM_STATE = "state"; // GET-Parameter "state"
    const String PARAM_VALUE = "value"; // GET-Parameter "value"

    const std::vector<PWMSwitch *> &switches; // Vector of switches
    std::unique_ptr<AsyncWebServer> server;   // WebServer for requests
    AwsTemplateProcessor processor;           // Processor for Strings
};

#endif // SWITCH_WEB_SERVER_H
