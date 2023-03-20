#ifndef _WEB_SERVER_H_
#define _WEB_SERVER_H_
#pragma once

#include "esp_http_server.h"

#ifdef __cplusplus
extern "C" {
#endif

class CWebServer
{
public:
    CWebServer();
    virtual ~CWebServer();
    static CWebServer* Instance();

public:
    bool start();
    bool stop();

private:
    static CWebServer* _instance;
    httpd_handle_t m_handle;

    void init_spiffs();

    bool register_uri_handler_get_common();
    static esp_err_t uri_handler_get_common(httpd_req_t *req);
    bool register_uri_handler_get_dpot_state();
    static esp_err_t uri_handler_get_dpot_state(httpd_req_t *req);
    bool register_uri_handler_post_dpot_config();
    static esp_err_t uri_handler_post_dpot_config(httpd_req_t *req);
    bool register_uri_handler_get_ws2812_state();
    static esp_err_t uri_handler_get_ws2812_state(httpd_req_t *req);
    bool register_uri_handler_post_ws2812_config();
    static esp_err_t uri_handler_post_ws2812_config(httpd_req_t *req);
    bool register_uri_handler_post_ws2812_blink();
    static esp_err_t uri_handler_post_ws2812_blink(httpd_req_t *req);
};

inline CWebServer* GetWebServer() {
    return CWebServer::Instance();
}

#ifdef __cplusplus
};
#endif
#endif