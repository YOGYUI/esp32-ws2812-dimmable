/**
 * @file webserver.cpp
 * @author yogyui
 * @brief web hosting server implementation
 * @version 0.1
 * @date 2023-03-14
 * 
 * @copyright Copyright (c) 2023
 */
#include "webserver.h"
#include "logger.h"
#include "definition.h"
#include <fcntl.h>
#include "esp_system.h"
#include "esp_spiffs.h"
#include "esp_vfs.h"
#include "cJSON.h"
#include "ws2812.h"
#include "dpotctrl.h"

#define FILE_PATH_MAX                       ESP_VFS_PATH_MAX + 128
#define SCRATCH_BUFSIZE                     10240
#define CHECK_FILE_EXTENSION(filename, ext) (strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0)
#define SPIFFS_BASE_PATH                    "/spiffs"
#define PARTITION_LABEL                     "web"

static char buffer[SCRATCH_BUFSIZE]{};
CWebServer* CWebServer::_instance = nullptr;

CWebServer::CWebServer()
{
    m_handle = nullptr;
    init_spiffs();
}

CWebServer::~CWebServer()
{
}

CWebServer* CWebServer::Instance()
{
    if (!_instance) {
        _instance = new CWebServer();
    }

    return _instance;
}

void CWebServer::init_spiffs()
{
    esp_vfs_spiffs_conf_t conf;
    conf.base_path = SPIFFS_BASE_PATH;      // File path prefix associated with the filesystem.
    conf.partition_label = PARTITION_LABEL; // Optional, label of SPIFFS partition to use. If set to NULL, first partition with subtype=spiffs will be used.
    conf.max_files = 5;                     // Maximum files that could be open at the same time.
    conf.format_if_mount_failed = false;    // If true, it will format the file system if it fails to mount.
    esp_err_t result = esp_vfs_spiffs_register(&conf);
    if (result != ESP_OK) {
        if (result == ESP_FAIL) {
            GetLogger(eLogType::Error)->Log("Failed to mount or format filesystem");
        } else if (result == ESP_ERR_NOT_FOUND) {
            GetLogger(eLogType::Error)->Log("Failed to find SPIFFS partition");
        } else {
            GetLogger(eLogType::Error)->Log("Failed to initialize SPIFFS (%s)", esp_err_to_name(result));
        }
        return;
    }

    size_t total = 0, used = 0;
    result = esp_spiffs_info(PARTITION_LABEL, &total, &used);
    if (result != ESP_OK) {
        GetLogger(eLogType::Error)->Log("Failed to get SPIFFS partition information (%s)", esp_err_to_name(result));
        return;
    } else {
        GetLogger(eLogType::Info)->Log("Partition size: total: %d, used: %d", total, used);
    }

    GetLogger(eLogType::Info)->Log("initialized SPIFFS");
}

bool CWebServer::start()
{
    stop();

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = WEB_SERVER_PORT;
    config.uri_match_fn = httpd_uri_match_wildcard;

    GetLogger(eLogType::Info)->Log("Starting HTTP Server (port %d)", config.server_port);
    esp_err_t result = httpd_start(&m_handle, &config);
    if (result != ESP_OK) {
        GetLogger(eLogType::Error)->Log("Failed to start HTTP Server (ret: %d)", result);
        stop();
        return false;
    }

    register_uri_handler_get_dpot_state();
    register_uri_handler_post_dpot_config();
    register_uri_handler_get_ws2812_state();
    register_uri_handler_post_ws2812_config();
    register_uri_handler_post_ws2812_blink();
    register_uri_handler_get_common();
    
    GetLogger(eLogType::Info)->Log("Started");
    return true;
}

bool CWebServer::stop()
{
    if (m_handle) {
        esp_err_t result = httpd_stop(m_handle);
        if (result != ESP_OK) {
            GetLogger(eLogType::Error)->Log("Failed to stop HTTP Server (ret: %d)", result);
            m_handle = nullptr;
            return false;
        }
        m_handle = nullptr;
        GetLogger(eLogType::Info)->Log("Stopped");
    }

    return true;
}

bool CWebServer::register_uri_handler_get_common()
{
    httpd_uri_t conf;
    conf.uri = "/*";
    conf.method = HTTP_GET;
    conf.handler = CWebServer::uri_handler_get_common;
    conf.user_ctx = nullptr;

    if (!m_handle) {
        GetLogger(eLogType::Error)->Log("Server is not started");
        return false;
    }

    esp_err_t result = httpd_register_uri_handler(m_handle, &conf);
    if (result != ESP_OK) {
        GetLogger(eLogType::Error)->Log("Failed to register uri handler %s (ret: %d)", conf.uri, result);
        return false;
    }

    return true;
}

esp_err_t CWebServer::uri_handler_get_common(httpd_req_t *req)
{
    char filepath[FILE_PATH_MAX]{};
    char message[256]{};
    snprintf(filepath, sizeof(filepath), SPIFFS_BASE_PATH);   // base-path

    // set file path from uri 
    // vue router path will not add trailing slash!
    std::string uri_str = std::string(req->uri);
    std::string extension = uri_str.substr(uri_str.find_last_of(".") + 1);
    if (extension.at(0) == '/') {
        strlcat(filepath, "/index.html", sizeof(filepath));
    } else {
        strlcat(filepath, req->uri, sizeof(filepath));
    }

    // open file
    int fd = open(filepath, O_RDONLY, 0);
    if (fd == -1) {
        snprintf(message, sizeof(message), "Failed to read existing file (%s)", filepath);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, message);
        return ESP_FAIL;
    }

    // set http type
    const char *type = "text/plain";
    if (CHECK_FILE_EXTENSION(filepath, ".html")) {
        type = "text/html";
    } else if (CHECK_FILE_EXTENSION(filepath, ".js")) {
        type = "application/javascript";
    } else if (CHECK_FILE_EXTENSION(filepath, ".css")) {
        type = "text/css";
    } else if (CHECK_FILE_EXTENSION(filepath, ".png")) {
        type = "image/png";
    } else if (CHECK_FILE_EXTENSION(filepath, ".ico")) {
        type = "image/x-icon";
    } else if (CHECK_FILE_EXTENSION(filepath, ".svg")) {
        type = "text/xml";
    }
    httpd_resp_set_type(req, type);

    // read file
    ssize_t read_bytes;
    while (true) {
        read_bytes = read(fd, buffer, SCRATCH_BUFSIZE);
        if (read_bytes == -1) {
            GetLogger(eLogType::Error)->Log("Failed to read file (%s)", filepath);
            break;
        } else if (read_bytes == 0) {
            break;
        } else if (read_bytes > 0) {
            // send buffer
            if (httpd_resp_send_chunk(req, buffer, read_bytes) != ESP_OK) {
                close(fd);
                GetLogger(eLogType::Error)->Log("Failed to send buffer");
                httpd_resp_sendstr_chunk(req, nullptr); // abort sending file
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
                return ESP_FAIL;
            }
        }
    }
    close(fd);
    // response empty chunk (reason: response completion)
    httpd_resp_send_chunk(req, NULL, 0);

    return ESP_OK;
}

bool CWebServer::register_uri_handler_get_dpot_state()
{
    httpd_uri_t conf;
    conf.uri = "/api/v1/dpot/state";
    conf.method = HTTP_GET;
    conf.handler = CWebServer::uri_handler_get_dpot_state;
    conf.user_ctx = nullptr;

    if (!m_handle) {
        GetLogger(eLogType::Error)->Log("Server is not started");
        return false;
    }

    esp_err_t result = httpd_register_uri_handler(m_handle, &conf);
    if (result != ESP_OK) {
        GetLogger(eLogType::Error)->Log("Failed to register uri handler %s (ret: %d)", conf.uri, result);
        return false;
    }

    return true;
}

esp_err_t CWebServer::uri_handler_get_dpot_state(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/json");

    cJSON *root = cJSON_CreateObject();
    if (root) {
        cJSON_AddNumberToObject(root, "raw_value", GetDPotCtrl()->get_raw_value());
        const char *info = cJSON_Print(root);
        httpd_resp_sendstr(req, info);
        free((void *)info);
        cJSON_Delete(root);
    }

    return ESP_OK;
}

bool CWebServer::register_uri_handler_post_dpot_config()
{
    httpd_uri_t conf;
    conf.uri = "/api/v1/dpot/config";
    conf.method = HTTP_POST;
    conf.handler = CWebServer::uri_handler_post_dpot_config;
    conf.user_ctx = nullptr;

    if (!m_handle) {
        GetLogger(eLogType::Error)->Log("Server is not started");
        return false;
    }

    esp_err_t result = httpd_register_uri_handler(m_handle, &conf);
    if (result != ESP_OK) {
        GetLogger(eLogType::Error)->Log("Failed to register uri handler %s (ret: %d)", conf.uri, result);
        return false;
    }

    return true;
}

esp_err_t CWebServer::uri_handler_post_dpot_config(httpd_req_t *req)
{
    char *buf = new char[req->content_len + 1];
    size_t offset = 0;
    int ret;

    if (!buf) {
        GetLogger(eLogType::Error)->Log("Failed to allocate buffer");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    while (offset < req->content_len) {
        ret = httpd_req_recv(req, buf + offset, req->content_len - offset);
        if (ret <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                httpd_resp_send_408(req);
            }
            delete[] buf;
            return ESP_FAIL;
        }
        offset += ret;
    }
    buf[offset] = '\0';

    cJSON *item = cJSON_ParseWithLength(buf, offset);
    delete[] buf;

    if (item) {
        const cJSON *item_raw_value = cJSON_GetObjectItemCaseSensitive(item, "raw_value");
        if (item_raw_value) {
            uint8_t value = (uint8_t)item_raw_value->valuedouble;

            // GetLogger(eLogType::Info)->Log("dpot post type %d raw value: %g (%d)", item_raw_value->type, item_raw_value->valuedouble, value);
            if (GetDPotCtrl()->set_raw_value(value)) {
                httpd_resp_set_status(req, HTTPD_200);
                httpd_resp_send(req, "OK", 3);
            } else {
                httpd_resp_set_status(req, HTTPD_500);
                httpd_resp_send(req, "NG", 3);
            }
        }

        cJSON_Delete(item);
    }

    return ESP_OK;
}

bool CWebServer::register_uri_handler_get_ws2812_state()
{
    httpd_uri_t conf;
    conf.uri = "/api/v1/ws2812/state";
    conf.method = HTTP_GET;
    conf.handler = CWebServer::uri_handler_get_ws2812_state;
    conf.user_ctx = nullptr;

    if (!m_handle) {
        GetLogger(eLogType::Error)->Log("Server is not started");
        return false;
    }

    esp_err_t result = httpd_register_uri_handler(m_handle, &conf);
    if (result != ESP_OK) {
        GetLogger(eLogType::Error)->Log("Failed to register uri handler %s (ret: %d)", conf.uri, result);
        return false;
    }

    return true;
}

esp_err_t CWebServer::uri_handler_get_ws2812_state(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/json");

    cJSON *root = cJSON_CreateObject();
    if (root) {
        cJSON_AddNumberToObject(root, "brightness", GetWS2812Ctrl()->get_brightness());
        RGB rgb = GetWS2812Ctrl()->get_common_color();
        cJSON_AddNumberToObject(root, "red", rgb.r);
        cJSON_AddNumberToObject(root, "green", rgb.g);
        cJSON_AddNumberToObject(root, "blue", rgb.b);
        const char *info = cJSON_Print(root);
        httpd_resp_sendstr(req, info);
        free((void *)info);
        cJSON_Delete(root);
    }

    return ESP_OK;
}

bool CWebServer::register_uri_handler_post_ws2812_config()
{
    httpd_uri_t conf;
    conf.uri = "/api/v1/ws2812/config";
    conf.method = HTTP_POST;
    conf.handler = CWebServer::uri_handler_post_ws2812_config;
    conf.user_ctx = nullptr;

    if (!m_handle) {
        GetLogger(eLogType::Error)->Log("Server is not started");
        return false;
    }

    esp_err_t result = httpd_register_uri_handler(m_handle, &conf);
    if (result != ESP_OK) {
        GetLogger(eLogType::Error)->Log("Failed to register uri handler %s (ret: %d)", conf.uri, result);
        return false;
    }

    return true;
}

esp_err_t CWebServer::uri_handler_post_ws2812_config(httpd_req_t *req)
{
    char *buf = new char[req->content_len + 1];
    size_t offset = 0;
    int ret;

    if (!buf) {
        GetLogger(eLogType::Error)->Log("Failed to allocate buffer");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    while (offset < req->content_len) {
        ret = httpd_req_recv(req, buf + offset, req->content_len - offset);
        if (ret <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                httpd_resp_send_408(req);
            }
            delete[] buf;
            return ESP_FAIL;
        }
        offset += ret;
    }
    buf[offset] = '\0';

    cJSON *item = cJSON_ParseWithLength(buf, offset);
    delete[] buf;

    if (item) {
        const cJSON *item_brightness = cJSON_GetObjectItemCaseSensitive(item, "brightness");
        if (item_brightness) {
            uint8_t value = (uint8_t)(item_brightness->valuedouble);
            if (GetWS2812Ctrl()->set_brightness(value)) {
                httpd_resp_set_status(req, HTTPD_200);
                httpd_resp_send(req, "OK", 3);
            } else {
                httpd_resp_set_status(req, HTTPD_500);
                httpd_resp_send(req, "NG", 3);
            }
        }

        const cJSON *item_rgb = cJSON_GetObjectItemCaseSensitive(item, "rgb");
        if (item_rgb) {
            uint8_t r = 0, g = 0, b = 0;
            cJSON *elem;
            int arr_size = cJSON_GetArraySize(item_rgb);
            if (arr_size >= 1) {
                elem = cJSON_GetArrayItem(item_rgb, 0);
                r = (uint8_t)(elem->valuedouble);
            }
            if (arr_size >= 2) {
                elem = cJSON_GetArrayItem(item_rgb, 1);
                g = (uint8_t)(elem->valuedouble);
            }
            if (arr_size >= 3) {
                elem = cJSON_GetArrayItem(item_rgb, 2);
                b = (uint8_t)(elem->valuedouble);
            }

            if (GetWS2812Ctrl()->set_common_color(r, g, b)) {
                httpd_resp_set_status(req, HTTPD_200);
                httpd_resp_send(req, "OK", 3);
            } else {
                httpd_resp_set_status(req, HTTPD_500);
                httpd_resp_send(req, "NG", 3);
            }
        }

        cJSON_Delete(item);
    }

    return ESP_OK;
}

bool CWebServer::register_uri_handler_post_ws2812_blink()
{
    httpd_uri_t conf;
    conf.uri = "/api/v1/ws2812/blink";
    conf.method = HTTP_POST;
    conf.handler = CWebServer::uri_handler_post_ws2812_blink;
    conf.user_ctx = nullptr;

    if (!m_handle) {
        GetLogger(eLogType::Error)->Log("Server is not started");
        return false;
    }

    esp_err_t result = httpd_register_uri_handler(m_handle, &conf);
    if (result != ESP_OK) {
        GetLogger(eLogType::Error)->Log("Failed to register uri handler %s (ret: %d)", conf.uri, result);
        return false;
    }

    return true;
}

esp_err_t CWebServer::uri_handler_post_ws2812_blink(httpd_req_t *req)
{
    char *buf = new char[req->content_len + 1];
    size_t offset = 0;
    int ret;

    if (!buf) {
        GetLogger(eLogType::Error)->Log("Failed to allocate buffer");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    while (offset < req->content_len) {
        ret = httpd_req_recv(req, buf + offset, req->content_len - offset);
        if (ret <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                httpd_resp_send_408(req);
            }
            delete[] buf;
            return ESP_FAIL;
        }
        offset += ret;
    }
    buf[offset] = '\0';

    cJSON *item = cJSON_ParseWithLength(buf, offset);
    delete[] buf;

    if (item) {
        uint32_t duration = 1000;
        uint32_t count = 1;
        const cJSON *item_duration = cJSON_GetObjectItemCaseSensitive(item, "duration");
        if (item_duration) {
            duration = (uint8_t)(item_duration->valuedouble);
        }
        const cJSON *item_count = cJSON_GetObjectItemCaseSensitive(item, "count");
        if (item_count) {
            count = (uint8_t)(item_count->valuedouble);
        }

        if (GetWS2812Ctrl()->blink(duration, count)) {
            httpd_resp_set_status(req, HTTPD_200);
            httpd_resp_send(req, "OK", 3);
        } else {
            httpd_resp_set_status(req, HTTPD_500);
            httpd_resp_send(req, "NG", 3);
        }

        cJSON_Delete(item);
    }

    return ESP_OK;
}
