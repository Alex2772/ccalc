//
// Created by alex2772 on 18.11.18.
//

#include <sstream>
#include <esp_http_client.h>
#include <cstring>
#include "Http.h"
#include "Wifi.h"


http::Document http::get(const std::string &url) {
    return get(url, {});
}
std::string urlencode(const std::string &c) {

    std::string escaped = "";
    int max = c.length();
    for (int i = 0; i < max; i++) {
        if ((48 <= c[i] && c[i] <= 57) ||//0-9
            (65 <= c[i] && c[i] <= 90) ||//abc...xyz
            (97 <= c[i] && c[i] <= 122) || //ABC...XYZ
            (c[i] == '~' || c[i] == '!' || c[i] == '*' || c[i] == '(' || c[i] == ')' || c[i] == '\'' || c[i] == '_')
                ) {
            escaped.append(&c[i], 1);
        } else {
            escaped.append("%");
            char buf[4];
            sprintf(buf, "%02x", c[i]);
            escaped.append(buf);//converts char 255 to string "ff"
        }
    }
    return escaped;
}
http::Document http::get(const std::string &url, const std::map<std::string, std::string> &args) {
    esp_err_t err = -1;
    if (Wifi::isConnected()) {
        std::string finalUrl;
        {
            std::stringstream ss;
            ss << url;
            if (!args.empty()) {
                ss << "?";
                /*
                for (std::map<std::string, std::string>::const_iterator i = args.begin(); i != args.end(); ++i) {

                }*/
                bool first = true;
                for (auto &s : args) {
                    if (first) {
                        first = false;
                    } else {
                        ss << '&';
                    }
                    ss << s.first << '=' << urlencode(s.second);
                }
            }

            finalUrl = ss.str();
        }
        printf("HTTP %s\n", finalUrl.c_str());
        esp_http_client_config_t cfg;
        memset(&cfg, 0, sizeof(cfg));
        cfg.method = HTTP_METHOD_GET;
        cfg.url = finalUrl.c_str();
        esp_http_client_handle_t client = esp_http_client_init(&cfg);
        err = esp_http_client_open(client, 0);
        if (err == ESP_OK) {
            char buffer[4096];
            int read = 0;
            std::stringstream ss;
            int contentLength = esp_http_client_fetch_headers(client);
            if (contentLength) {
                do {
                    int readNow = esp_http_client_read(client, buffer, sizeof(buffer));
                    ss.write(buffer, readNow);
                    read += readNow;
                } while (read < contentLength);
            } else {
                while ((read = esp_http_client_read(client, buffer, sizeof(buffer)))) {
                    ss.write(buffer, read);
                }
            }
            http::Document d = {esp_http_client_get_status_code(client), ss.str()};
            printf("HTTP done, %d\n", esp_http_client_get_status_code(client));
            esp_http_client_close(client);
            esp_http_client_cleanup(client);
            return d;
        }
        esp_http_client_cleanup(client);
    }
    return {err, std::string()};
}
esp_err_t _http_event_handle(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ON_HEADER: {
            http::Document* doc = reinterpret_cast<http::Document*>(evt->custom_data);
            doc->mHeaders[evt->header_key] = evt->header_value;
            break;
        }
        case HTTP_EVENT_ERROR:break;
        case HTTP_EVENT_ON_CONNECTED:break;
        case HTTP_EVENT_HEADER_SENT:break;
        case HTTP_EVENT_ON_DATA:break;
        case HTTP_EVENT_ON_FINISH:break;
        case HTTP_EVENT_DISCONNECTED:break;
    }
    return ESP_OK;
}

http::Document http::get(const std::string &url, const std::map<std::string, std::string> &args, const std::function<void(char* buf, int len, int contentLength)>& callback) {
    esp_err_t err = -1;
    http::Document doc = {};
    if (Wifi::isConnected()) {
        std::string finalUrl;
        {
            std::stringstream ss;
            ss << url;
            if (!args.empty()) {
                ss << "?";
                /*
                for (std::map<std::string, std::string>::const_iterator i = args.begin(); i != args.end(); ++i) {

                }*/
                bool first = true;
                for (auto &s : args) {
                    if (first) {
                        first = false;
                    } else {
                        ss << '&';
                    }
                    ss << s.first << '=' << urlencode(s.second);
                }
            }

            finalUrl = ss.str();
        }
        printf("HTTP %s\n", finalUrl.c_str());
        esp_http_client_config_t cfg = {};
        cfg.method = HTTP_METHOD_GET;
        cfg.url = finalUrl.c_str();
        cfg.event_handler = _http_event_handle;
        cfg.custom_data = reinterpret_cast<char*>(&doc);
        esp_http_client_handle_t client = esp_http_client_init(&cfg);

        err = esp_http_client_open(client, 0);
        if (err == ESP_OK) {
            char buffer[4096];
            int read = 0;

            int contentLength = esp_http_client_fetch_headers(client);
            if (contentLength) {
                do {
                    int readNow = esp_http_client_read(client, buffer, sizeof(buffer));
                    callback(buffer, readNow, contentLength);
                    read += readNow;
                } while (read < contentLength);
            } else {
                while ((read = esp_http_client_read(client, buffer, sizeof(buffer)))) {
                    callback(buffer, read, contentLength);
                }
            }
            printf("HTTP done, %d\n", doc.mStatus = esp_http_client_get_status_code(client));
            esp_http_client_close(client);
            esp_http_client_cleanup(client);

            return doc;
        }
        esp_http_client_cleanup(client);
    }
    return {-1};
}
