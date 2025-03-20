#include <Arduino.h>
#include "esp_http_server.h"
#include "esp_camera.h"
#include "camera_index.h"

// ✅ Function Declarations (Ensures Compiler Knows About These)
esp_err_t index_handler(httpd_req_t *req);
esp_err_t secret_handler(httpd_req_t *req);
esp_err_t handle_jpg_stream(httpd_req_t *req);

// ✅ Global Variables
httpd_handle_t stream_httpd = NULL;
bool detectGoodOil = true;
bool detectBadOil = true;

// ✅ Define Good Oil Colors (Golden, Yellow, Greenish)
const uint32_t GOOD_OIL_COLORS[] = {
    0xB5A642, // Olive Oil (Extra Virgin)
    0xE1C16E, // Olive Oil (Refined)
    0xF3D673, // Vegetable Oil
    0xF4E3B2, // Canola Oil
    0xF7D86F, // Sunflower Oil
    0xF6C453, // Corn Oil
    0xEEC373, // Peanut Oil
    0xD9A441, // Sesame Oil (Light)
    0xFFD700, // Palm Oil (Refined)
    0x6A8A2F, // Avocado Oil
    0xC0D860  // Grapeseed Oil
};

// ✅ Define Bad Oil Colors (Dark Brown, Burnt)
const uint32_t BAD_OIL_COLORS[] = {
    0x5A3E2B, // Burnt Oil
    0xA35C2B, // Heavily Used Oil
    0xC68942, // Moderately Used Oil
    0xE1B96A  // Lightly Used Oil
};

// ✅ Function to Check if a Pixel is "Good Oil"
bool isGoodOil(uint16_t color) {
    if (!detectGoodOil) return false;
    uint32_t colorHex = ((color >> 8) & 0xF8) << 16 | ((color >> 3) & 0xFC) << 8 | (color << 3);
    for (int i = 0; i < sizeof(GOOD_OIL_COLORS) / sizeof(GOOD_OIL_COLORS[0]); i++) {
        if (colorHex == GOOD_OIL_COLORS[i]) {
            return true;
        }
    }
    return false;
}

// ✅ Function to Check if a Pixel is "Bad Oil"
bool isBadOil(uint16_t color) {
    if (!detectBadOil) return false;
    uint32_t colorHex = ((color >> 8) & 0xF8) << 16 | ((color >> 3) & 0xFC) << 8 | (color << 3);
    for (int i = 0; i < sizeof(BAD_OIL_COLORS) / sizeof(BAD_OIL_COLORS[0]); i++) {
        if (colorHex == BAD_OIL_COLORS[i]) {
            return true;
        }
    }
    return false;
}

// ✅ Function to Process Camera Frame and Detect Oil Colors
esp_err_t handle_oil_detection(httpd_req_t *req) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        return ESP_FAIL;
    }

    uint16_t *image = (uint16_t*)fb->buf;
    int countGoodOil = 0, countBadOil = 0;

    for (int i = 0; i < (fb->len / 2); i++) {
        if (isGoodOil(image[i])) countGoodOil++;
        if (isBadOil(image[i])) countBadOil++;
    }

    esp_camera_fb_return(fb);

    char response[100];
    sprintf(response, "{\"good_oil\": %d, \"bad_oil\": %d}", countGoodOil, countBadOil);
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, response, strlen(response));
}

// ✅ Function to Enable/Disable Oil Detection
esp_err_t set_oil_mode(httpd_req_t *req) {
    char param[32];
    if (httpd_query_key_value(req->uri, "oil", param, sizeof(param)) == ESP_OK) {
        if (strcmp(param, "good") == 0) {
            detectGoodOil = true;
            detectBadOil = false;
        } else if (strcmp(param, "bad") == 0) {
            detectGoodOil = false;
            detectBadOil = true;
        } else {
            detectGoodOil = false;
            detectBadOil = false;
        }
        Serial.printf("Oil mode set: Good(%d), Bad(%d)\n", detectGoodOil, detectBadOil);
    }
    return httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
}

// ✅ Function to Serve the Main Webpage
esp_err_t index_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, (const char*)INDEX_HTML, strlen(INDEX_HTML));
}

// ✅ Function to Serve the Secret Page
esp_err_t secret_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, (const char*)SECRET_HTML, strlen(SECRET_HTML));
}

// ✅ Function to Handle Video Stream
esp_err_t handle_jpg_stream(httpd_req_t *req) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_send(req, (const char*)fb->buf, fb->len);
    esp_camera_fb_return(fb);
    return ESP_OK;
}

// ✅ Start Camera Web Server
void startCameraServer() { 
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    static httpd_uri_t uri_index = {"/", HTTP_GET, index_handler, NULL};
    static httpd_uri_t uri_secret = {"/secret", HTTP_GET, secret_handler, NULL};
    static httpd_uri_t uri_oil = {"/setoil", HTTP_GET, set_oil_mode, NULL};
    static httpd_uri_t uri_detect = {"/detect", HTTP_GET, handle_oil_detection, NULL};
    static httpd_uri_t uri_stream = {"/stream", HTTP_GET, handle_jpg_stream, NULL}; 

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &uri_index);
        httpd_register_uri_handler(server, &uri_secret);
        httpd_register_uri_handler(server, &uri_oil);
        httpd_register_uri_handler(server, &uri_detect);
        httpd_register_uri_handler(server, &uri_stream);
    }
}
