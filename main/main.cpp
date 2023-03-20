/**
 * @file main.cpp
 * @author yogyui
 * @brief 
 * @version 0.1
 * @date 2023-03-14
 * 
 * @copyright Copyright (c) 2023
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "definition.h"
#include "logger.h"
#include "ws2812.h"
#include "network.h"
#include "webserver.h"
#include "memory.h"

extern "C" void app_main(void)
{
    esp_err_t err;

    err = esp_netif_init();
    if (err != ESP_OK) {
        GetLogger(eLogType::Error)->Log("Failed to initialize network interface (ret: %d)", err);
    }

    err = esp_event_loop_create_default();
    if (err != ESP_OK) {
        GetLogger(eLogType::Error)->Log("Failed to create event loop (ret: %d)", err);
    }

    activate_wifi_softap();
    GetWS2812Ctrl()->initialize(PIN_WS2812_DATA, WS2812_PIXEL_COUNT);

    uint8_t brightness = 0;
    GetMemory()->load_ws2812_brightness(&brightness);
    GetWS2812Ctrl()->set_brightness(brightness);

    uint8_t red = 0, green = 0, blue = 0;
    GetMemory()->load_ws2812_color(&red, &green, &blue);
    GetWS2812Ctrl()->set_common_color(red, green, blue);
    
    GetWebServer()->start();
}
