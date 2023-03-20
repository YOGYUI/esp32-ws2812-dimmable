#ifndef _DEFINITION_H_
#define _DEFINITION_H_
#pragma once

// DPOT Parameters
#define DPOT_SPI_HOST           HSPI_HOST
#define PIN_DPOT_SPI_MOSI       13
#define PIN_DPOT_SPI_MISO       12
#define PIN_DPOT_SPI_SCLK       14
#define PIN_DPOT_SPI_CS         15

#define DPOT_RAB_RESISTANCE     10000   // AD5160 resistance between A-B terminal, 10Kohm
#define DPOT_RW_RESISTANCE      60      // AD5160 wiper resistance

// WS2812 Parameters
#define PIN_WS2812_PWM          19
#define PIN_WS2812_DATA         18

#define WS2812_PIXEL_COUNT      16
#define TASK_PRIORITY_WS2812    10
#define LED_SET_ALL             -1

// PWM Parameters
#define LED_PWM_FREQUENCY       200
#define PWM_DUTY_MAX            500

// Web Server & Network
#define WEB_SERVER_PORT         80
#define WIFI_SSID               "YOGYUI-ESP32-TEST"

#endif
