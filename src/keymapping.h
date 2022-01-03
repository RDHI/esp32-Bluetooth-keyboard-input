#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp_bt_defs.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"

#include "esp_hidh.h"
#include "esp_hid_gap.h"

extern xQueueHandle btkey_queue;

typedef struct {
    uint8_t keyboard_scancode;
    char ascii_key[10];
} btkey;

#define BT_KEY_MAX 54

// https://cdn.sparkfun.com/datasheets/Wireless/Bluetooth/RN-HID-User-Guide-v1.0r.pdf

void key_pressed(uint8_t* data, size_t size);
void init_btkey_queue();
uint16_t Key2code(uint8_t keyboard_scancode, bool uppercase, bool IsAscii);
