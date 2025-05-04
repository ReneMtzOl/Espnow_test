#include "espnow_manager.h"
#include "espnow_config.h"

#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "string.h"

#define TAG "ESPNOW_MANAGER"

typedef struct {
    uint8_t mac_addr[6];
    uint8_t data[250];
    int len;
} espnow_packet_t;

static QueueHandle_t send_queue;
static QueueHandle_t recv_queue;

static void espnow_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int len) {
    if (!mac_addr || !data || len <= 0) return;

    espnow_packet_t packet;
    memcpy(packet.mac_addr, mac_addr, 6);
    memcpy(packet.data, data, len);
    packet.len = len;

    xQueueSend(recv_queue, &packet, portMAX_DELAY);
}

esp_err_t espnow_manager_send(const uint8_t *peer_mac, const uint8_t *data, int len) {
    return esp_now_send(peer_mac, data, len);
}

static void receiver_task(void *arg) {
    espnow_packet_t packet;
    while (true) {
        if (xQueueReceive(recv_queue, &packet, portMAX_DELAY)) {
            ESP_LOGI(TAG, "Received %d bytes from %02x:%02x:%02x:%02x:%02x:%02x",
                     packet.len,
                     packet.mac_addr[0], packet.mac_addr[1], packet.mac_addr[2],
                     packet.mac_addr[3], packet.mac_addr[4], packet.mac_addr[5]);
        }
    }
}

esp_err_t espnow_manager_init(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_ERROR_CHECK(esp_now_init());
    //ESP_ERROR_CHECK(esp_now_register_recv_cb(espnow_recv_cb));

    ESP_ERROR_CHECK(esp_now_set_pmk((uint8_t *)ESPNOW_PMK));

    recv_queue = xQueueCreate(ESPNOW_RECV_QUEUE_SIZE, sizeof(espnow_packet_t));
    xTaskCreate(receiver_task, "espnow_receiver", 4096, NULL, 5, NULL);

    return ESP_OK;
}