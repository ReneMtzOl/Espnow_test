idf_component_register(SRCS "espnow_manager.c"
                       INCLUDE_DIRS "include"
                       REQUIRES esp_event esp_netif esp_wifi)