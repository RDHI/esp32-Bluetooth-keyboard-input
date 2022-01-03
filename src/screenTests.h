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
#include "bmpfile.h"
#include "pngle.h"
#include "decode_image.h"
#include "esp_timer.h"
#include "esp_sleep.h"
#include "esp_sntp.h"
TickType_t FillTest(TFT_t * dev, int width, int height);
TickType_t ColorBarTest(TFT_t * dev, int width, int height);
TickType_t ArrowTest(TFT_t * dev, FontxFile *fx, int width, int height);
TickType_t DirectionTest(TFT_t * dev, FontxFile *fx, int width, int height);
TickType_t HorizontalTest(TFT_t * dev, FontxFile *fx, int width, int height);
TickType_t VerticalTest(TFT_t * dev, FontxFile *fx, int width, int height);
TickType_t LineTest(TFT_t * dev, int width, int height);
TickType_t CircleTest(TFT_t * dev, int width, int height);
TickType_t RectAngleTest(TFT_t * dev, int width, int height);
TickType_t TriangleTest(TFT_t * dev, int width, int height);
TickType_t RoundRectTest(TFT_t * dev, int width, int height);
TickType_t FillRectTest(TFT_t * dev, int width, int height);
TickType_t ColorTest(TFT_t * dev, int width, int height);
TickType_t BMPTest(TFT_t * dev, char * file, int width, int height);
TickType_t JPEGTest(TFT_t * dev, char * file, int width, int height);
void png_init(pngle_t *pngle, uint32_t w, uint32_t h);
void png_draw(pngle_t *pngle, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t rgba[4]);
void png_finish(pngle_t *pngle);
TickType_t PNGTest(TFT_t * dev, char * file, int width, int height);
TickType_t CodeTest(TFT_t * dev, FontxFile *fx, int width, int height);