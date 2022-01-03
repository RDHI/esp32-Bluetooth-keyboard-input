#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_vfs.h"
#include "esp_spiffs.h"

#include "st7789.h"
#include "fontx.h"
#include "esp_timer.h"
#include "esp_sleep.h"
#include "esp_sntp.h"

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
#include "keymapping.h"
static const char *TAG = "MAIN";
#define SCAN_DURATION_SECONDS 5
#define NUM_FONT_FILES 2
#define NUM_COLORS 10
static int char_colors[NUM_COLORS] = {RED,GREEN,BLUE,BLACK,WHITE,GRAY,YELLOW,CYAN,PURPLE,ORANGE};

typedef struct {
	FontxFile FONT[2];
	uint8_t buffer[FontxGlyphBufSize];
	uint8_t fontWidth;
	uint8_t fontHeight;
} Fontparam_t;

static void SPIFFS_Directory(char * path) {
	DIR* dir = opendir(path);
	assert(dir != NULL);
	while (true) {
		struct dirent*pe = readdir(dir);
		if (!pe) break;
		ESP_LOGI(__FUNCTION__,"d_name=%s d_ino=%d d_type=%x", pe->d_name,pe->d_ino, pe->d_type);
	}
	closedir(dir);
}


void ST7789(void * pvParameters)
{	
	uint8_t reveived_scancode;
	uint16_t font_code;
	Fontparam_t *FontFiles;
	int fontcolor = CYAN;
	uint8_t Fontindex = 0;
	uint8_t Colorindex = 0;
	FontFiles = malloc(NUM_FONT_FILES*sizeof(Fontparam_t));
	if(FontFiles == NULL)
	{
	ESP_LOGE(__FUNCTION__, "Error allocating memory for font files");
	}
	// set font file
	InitFontx(FontFiles[0].FONT,"/spiffs/ILGH32XB.FNT",""); // 16x32Dot Gothic
	InitFontx(FontFiles[1].FONT,"/spiffs/test.fnt",""); // Chinese Chars

	TFT_t dev;
	spi_master_init(&dev, CONFIG_MOSI_GPIO, CONFIG_SCLK_GPIO, CONFIG_CS_GPIO, CONFIG_DC_GPIO, CONFIG_RESET_GPIO, CONFIG_BL_GPIO);
	lcdInit(&dev, CONFIG_WIDTH, CONFIG_HEIGHT, CONFIG_OFFSETX, CONFIG_OFFSETY);

#if CONFIG_INVERSION
	ESP_LOGI(TAG, "Enable Display Inversion");
	lcdInversionOn(&dev);
#endif
	// get font width & height
	// uint8_t buffer[FontxGlyphBufSize];
	GetFontx(FontFiles[0].FONT, 0, FontFiles[0].buffer, &FontFiles[0].fontWidth, &FontFiles[0].fontHeight);
	GetFontx(FontFiles[1].FONT, 0, FontFiles[1].buffer, &FontFiles[1].fontWidth, &FontFiles[1].fontHeight);
	uint8_t fontWidth = FontFiles[1].fontWidth;
	uint8_t fontHeight = FontFiles[1].fontHeight; // the w&h between font files are same
	//uint8_t xmoji = CONFIG_WIDTH / fontWidth;

	uint16_t xpos = 0;
	uint16_t ypos =  fontHeight - 1;

	bool uppercase= false;
	bool IsAscii = true;
	lcdFillScreen(&dev, BLACK);
	lcdSetFontDirection(&dev, 0);
	uint8_t CHI_index = 0;
	char CHI_inputsting[2];
	//bool lastinput = true;
	while(1){
	lcdDrawLine(&dev, xpos, ypos, xpos, ypos-fontHeight+1, fontcolor);	   
    if(xQueueReceive(btkey_queue, &reveived_scancode, 0)) {
		//new line if current line is full
		if (xpos >= CONFIG_WIDTH- fontWidth+1){
			lcdDrawLine(&dev, xpos, ypos, xpos, ypos-fontHeight+1, BLACK);
			xpos = 0;
			ypos += fontHeight;
		}
		//get font code
		font_code = Key2code(reveived_scancode, uppercase, IsAscii);
		//control codes
    	switch(font_code){
        case 0x102a: {
			lcdDrawLine(&dev, xpos, ypos, xpos, ypos-fontHeight+1, BLACK);
			if (xpos == 0 && ypos == fontHeight - 1) break;
			if (xpos!=0){
				xpos-=fontWidth;
				lcdDrawFillRect(&dev, xpos, ypos-fontHeight+1, xpos+fontWidth-1, ypos, BLACK);
				break;
			}
			else{
				xpos = CONFIG_WIDTH- fontWidth;
				ypos -= fontHeight;
				lcdDrawFillRect(&dev, xpos, ypos-fontHeight+1, xpos+fontWidth-1, ypos, BLACK);
				break;
			}
			// NOTE: backsapce might cause stackoverflow in task
			// the overflow should be fixed by increasing the stack size to 4096 in esp_ble_hidh_init or esp_bt_hidh_init
			// https://gitea.edwinclement08.com/espressif/esp-idf/commit/48311d6599d93dbf89d71bdb697df4c9bc620e52?style=split&whitespace=ignore-all
			// Also, this code will not catch upper left case.
		}; //backspace
        case 0x102b: {
			Fontindex = (Fontindex+1) % NUM_FONT_FILES;
			IsAscii = !IsAscii;
			break;
		}; //tab
        case 0x102c: {
			lcdDrawLine(&dev, xpos, ypos, xpos, ypos-fontHeight+1, BLACK);
			xpos+=fontWidth;
			break;
		}; //space
        case 0x1028: {
			lcdDrawLine(&dev, xpos, ypos, xpos, ypos-fontHeight+1, BLACK);
			xpos = 0;
			ypos += fontHeight;
			break;
		}; //return
		case 0x1029: {
			lcdFillScreen(&dev, BLACK);
			xpos = 0;
			ypos =  fontHeight - 1;
			ESP_LOGI(TAG,  "Clear screen");
			break;
		}; //return
        case 0x1039: {
			uppercase = !uppercase;
			break;
		}; //caps lock
		case 0x1001: {
		Colorindex = (Colorindex+1) % NUM_COLORS;
		fontcolor = char_colors[Colorindex];
		break;
		} //left control switch color
    }

		//regular ascii chars
		if (font_code <= 0xFF){
		lcdDrawLine(&dev, xpos, ypos, xpos, ypos-fontHeight+1, BLACK);
		xpos = lcdDrawCode(&dev, FontFiles[Fontindex].FONT, xpos, ypos, font_code, fontcolor);
		}
		//Chinese inputs
		else if (font_code > 0xFF00){
		
		CHI_inputsting[CHI_index] = (char)(font_code-0xff00);
		CHI_index = (CHI_index+1)%2;
		if (CHI_index == 0){
		font_code = (uint16_t)strtol(CHI_inputsting, NULL, 16);
		ESP_LOGI(TAG,  "str: %s fti:%d code:%04x", CHI_inputsting,Fontindex,font_code);
		lcdDrawLine(&dev, xpos, ypos, xpos, ypos-fontHeight+1, BLACK);
		xpos = lcdDrawCode(&dev, FontFiles[Fontindex].FONT, xpos, ypos, font_code, fontcolor);
		}
		}
    }
	vTaskDelay(50 / portTICK_PERIOD_MS); // eyes will not notice this :)
	}

	// never reach
	while (1) {
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
}

void hidh_callback(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
    esp_hidh_event_t event = (esp_hidh_event_t)id;
    esp_hidh_event_data_t *param = (esp_hidh_event_data_t *)event_data;

    switch (event) {
    case ESP_HIDH_OPEN_EVENT: {
        const uint8_t *bda = esp_hidh_dev_bda_get(param->open.dev);
        ESP_LOGI(TAG, ESP_BD_ADDR_STR " OPEN: %s", ESP_BD_ADDR_HEX(bda), esp_hidh_dev_name_get(param->open.dev));
        esp_hidh_dev_dump(param->open.dev, stdout);
        break;
    }
    case ESP_HIDH_BATTERY_EVENT: {
        const uint8_t *bda = esp_hidh_dev_bda_get(param->battery.dev);
        ESP_LOGI(TAG, ESP_BD_ADDR_STR " BATTERY: %d%%", ESP_BD_ADDR_HEX(bda), param->battery.level);
        break;
    }
    case ESP_HIDH_INPUT_EVENT: {
        const uint8_t *bda = esp_hidh_dev_bda_get(param->input.dev);
        ESP_LOGI(TAG, ESP_BD_ADDR_STR " INPUT: %8s, MAP: %2u, ID: %3u, Len: %d, Data:", ESP_BD_ADDR_HEX(bda), esp_hid_usage_str(param->input.usage), param->input.map_index, param->input.report_id, param->input.length);
        ESP_LOG_BUFFER_HEX(TAG, param->input.data, param->input.length);
		key_pressed(param->input.data, param->input.length);
        ESP_LOGI(TAG,  "Key Pressed");
        break;
    }
    case ESP_HIDH_FEATURE_EVENT: {
        const uint8_t *bda = esp_hidh_dev_bda_get(param->feature.dev);
        ESP_LOGI(TAG, ESP_BD_ADDR_STR " FEATURE: %8s, MAP: %2u, ID: %3u, Len: %d", ESP_BD_ADDR_HEX(bda), esp_hid_usage_str(param->feature.usage), param->feature.map_index, param->feature.report_id, param->feature.length);
        ESP_LOG_BUFFER_HEX(TAG, param->feature.data, param->feature.length);
        break;
    }
    case ESP_HIDH_CLOSE_EVENT: {
        const uint8_t *bda = esp_hidh_dev_bda_get(param->close.dev);
        ESP_LOGI(TAG, ESP_BD_ADDR_STR " CLOSE: '%s' %s", ESP_BD_ADDR_HEX(bda), esp_hidh_dev_name_get(param->close.dev), esp_hid_disconnect_reason_str(esp_hidh_dev_transport_get(param->close.dev), param->close.reason));
        //MUST call this function to free all allocated memory by this device
        esp_hidh_dev_free(param->close.dev);
        break;
    }
    default:
        ESP_LOGI(TAG, "EVENT: %d", event);
        break;
    }
}

void hid_demo_task(void *pvParameters)
{
    size_t results_len = 0;
    esp_hid_scan_result_t *results = NULL;
    ESP_LOGI(TAG, "SCAN...");
    //start scan for HID devices
    esp_hid_scan(SCAN_DURATION_SECONDS, &results_len, &results);
    ESP_LOGI(TAG, "SCAN: %u results", results_len);
    if (results_len) {
        esp_hid_scan_result_t *r = results;
        esp_hid_scan_result_t *cr = NULL;
        while (r) {
            printf("  %s: " ESP_BD_ADDR_STR ", ", (r->transport == ESP_HID_TRANSPORT_BLE) ? "BLE" : "BT ", ESP_BD_ADDR_HEX(r->bda));
            printf("RSSI: %d, ", r->rssi);
            printf("USAGE: %s, ", esp_hid_usage_str(r->usage));
            if (r->transport == ESP_HID_TRANSPORT_BLE) {
                cr = r;
                printf("APPEARANCE: 0x%04x, ", r->ble.appearance);
                printf("ADDR_TYPE: '%s', ", ble_addr_type_str(r->ble.addr_type));
            } else {
                cr = r;
                printf("COD: %s[", esp_hid_cod_major_str(r->bt.cod.major));
                esp_hid_cod_minor_print(r->bt.cod.minor, stdout);
                printf("] srv 0x%03x, ", r->bt.cod.service);
                print_uuid(&r->bt.uuid);
                printf(", ");
            }
            printf("NAME: %s ", r->name ? r->name : "");
            printf("\n");
            r = r->next;
        }
        if (cr) {
            //open the last result
            esp_hidh_dev_open(cr->bda, cr->transport, cr->ble.addr_type);
        }
        //free the results
        esp_hid_scan_results_free(results);
    }
    vTaskDelete(NULL);
}


void app_main(void)
{

	esp_err_t ret;

	init_btkey_queue();
	//spiffs
	ESP_LOGI(TAG, "Initializing SPIFFS");

	esp_vfs_spiffs_conf_t conf = {
		.base_path = "/spiffs",
		.partition_label = NULL,
		.max_files = 11,
		.format_if_mount_failed =true
	};

	// Use settings defined above toinitialize and mount SPIFFS filesystem.
	// Note: esp_vfs_spiffs_register is anall-in-one convenience function.
	ret = esp_vfs_spiffs_register(&conf);

	if (ret != ESP_OK) {
		if (ret == ESP_FAIL) {
			ESP_LOGE(TAG, "Failed to mount or format filesystem");
		} else if (ret == ESP_ERR_NOT_FOUND) {
			ESP_LOGE(TAG, "Failed to find SPIFFS partition");
		} else {
			ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)",esp_err_to_name(ret));
		}
		return;
	}

	size_t total = 0, used = 0;
	ret = esp_spiffs_info(NULL, &total,&used);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG,"Failed to get SPIFFS partition information (%s)",esp_err_to_name(ret));
	} else {
		ESP_LOGI(TAG,"Partition size: total: %d, used: %d", total, used);
	}

	SPIFFS_Directory("/spiffs/");
	
   	xTaskCreate(ST7789, "ST7789", 1024*12, NULL, 4, NULL);

    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );
	
	#if CONFIG_BT_CLASSIC_ENABLED
    ESP_ERROR_CHECK( esp_hid_gap_init(ESP_BT_MODE_BTDM) );
	#else
    ESP_ERROR_CHECK( esp_hid_gap_init(ESP_BT_MODE_BLE) );
	#endif
    ESP_ERROR_CHECK( esp_ble_gattc_register_callback(esp_hidh_gattc_event_handler) );
    esp_hidh_config_t config = {
        .callback = hidh_callback,
    }; //doesn't have the field task_stack_size
    ESP_ERROR_CHECK( esp_hidh_init(&config) );
	xTaskCreate(hid_demo_task, "hid_task", 10 * 1024, NULL, 2, NULL);


}
