#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t espnow_manager_init(void);
esp_err_t espnow_manager_send(const uint8_t *peer_mac, const uint8_t *data, int len);

#ifdef __cplusplus
}
#endif