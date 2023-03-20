/**
 * @file network.cpp
 * @author yogyui
 * @brief WIFI soft-AP configuration module
 * @version 0.1
 * @date 2023-03-14
 * 
 * @copyright Copyright (c) 2023
 */
#include "network.h"
#include "esp_wifi.h"
#include <nvs_flash.h>
#include "logger.h"
#include "definition.h"
#include <string.h>

#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        GetLogger(eLogType::Info)->Log("station %02x:%02x:%02x:%02x:%02x:%02x join, AID=%d", MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        GetLogger(eLogType::Info)->Log("station %02x:%02x:%02x:%02x:%02x:%02x leave, AID=%d", MAC2STR(event->mac), event->aid);
    }
}

bool activate_wifi_softap()
{
    esp_err_t err;
    wifi_mode_t cur_mode = WIFI_MODE_NULL;

    err = nvs_flash_init();
    if (err != ESP_OK) {
        GetLogger(eLogType::Error)->Log("Failed to initialize nvs flash (ret: %d)", err);
        return false;
    }

    esp_netif_t* wifiAP = esp_netif_create_default_wifi_ap();
    esp_netif_ip_info_t ipInfo;

    IP4_ADDR(&ipInfo.ip, 10, 11, 12, 1);
    IP4_ADDR(&ipInfo.gw, 0, 0, 0, 0); // do not advertise as a gateway router
    IP4_ADDR(&ipInfo.netmask, 255, 255, 255, 0);
    esp_netif_dhcps_stop(wifiAP);
    esp_netif_set_ip_info(wifiAP, &ipInfo);
    esp_netif_dhcps_start(wifiAP);

    wifi_init_config_t wifi_init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    err = esp_wifi_init(&wifi_init_cfg);
    if (err != ESP_OK) {
        GetLogger(eLogType::Error)->Log("Failed to initialize wifi (ret: %d)", err);
        return false;
    }

    err = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, nullptr, nullptr);
    
    err = esp_wifi_get_mode(&cur_mode);
    if (err != ESP_OK) {
        GetLogger(eLogType::Error)->Log("Failed to get wifi mode (ret: %d)", err);
        return false;
    }

    switch(cur_mode) {
    case WIFI_MODE_NULL:
        GetLogger(eLogType::Info)->Log("wifi-current mode: null");
        break;
    case WIFI_MODE_STA:
        GetLogger(eLogType::Info)->Log("wifi-current mode: station");
        break;
    case WIFI_MODE_AP:
        GetLogger(eLogType::Info)->Log("wifi-current mode: soft-ap");
        break;
    case WIFI_MODE_APSTA:
        GetLogger(eLogType::Info)->Log("wifi-current mode: station + soft-ap");
        break;
    default:
        break;
    }

    if (cur_mode & WIFI_MODE_AP) {
        GetLogger(eLogType::Info)->Log("wifi ap-mode is already activated");
    } else {
        err = esp_wifi_set_mode((wifi_mode_t)(cur_mode | WIFI_MODE_AP));
        if (err != ESP_OK) {
            GetLogger(eLogType::Error)->Log("Failed to set wifi mode (ret: %d)", err);
            return false;
        }
    }

    wifi_config_t wifi_config;
    memcpy(wifi_config.ap.ssid, WIFI_SSID, strlen(WIFI_SSID));
    memset(wifi_config.ap.password, 0, sizeof(wifi_config.ap.password));
    wifi_config.ap.ssid_len = strlen(WIFI_SSID);
    wifi_config.ap.channel = 1;
    wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    wifi_config.ap.ssid_hidden = 0;
    wifi_config.ap.max_connection = 10;
    wifi_config.ap.beacon_interval = 100;
    //wifi_config.ap.pairwise_cipher;
    //wifi_config.ap.ftm_responder;

    err = esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    if (err != ESP_OK) {
        GetLogger(eLogType::Error)->Log("Failed to set wifi config (ret: %d)", err);
        return false;
    }

    err = esp_wifi_start();
    if (err != ESP_OK) {
        GetLogger(eLogType::Error)->Log("Failed to start wifi");
        return false;
    }

    return true;
}
