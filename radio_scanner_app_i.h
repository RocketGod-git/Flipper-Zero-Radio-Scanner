#pragma once

#include "scenes/radio_scanner_scene.h"
#include "views/scanner.h"

#include <gui/gui.h>
#include <gui/modules/variable_item_list.h>
#include <gui/modules/widget.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <subghz/devices/devices.h>

#define TAG "RadioScannerApp"

#define RADIO_SCANNER_DEFAULT_FREQ        433920000
#define RADIO_SCANNER_DEFAULT_RSSI        (-100.0f)
#define RADIO_SCANNER_DEFAULT_SENSITIVITY (-85.0f)
#define RADIO_SCANNER_BUFFER_SZ           32

#define SUBGHZ_FREQUENCY_MIN  300000000
#define SUBGHZ_FREQUENCY_MAX  928000000
#define SUBGHZ_FREQUENCY_STEP 10000
#define SUBGHZ_DEVICE_NAME    "cc1101_int"

/**
 * Enumeration of view types used in the radio scanner app.
 */
typedef enum {
    RadioScannerViewScanner,
} RadioScannerView;

/**
 * Enumeration representing the scanning direction.
 */
typedef enum {
    ScanDirectionUp,
    ScanDirectionDown,
} ScanDirection;

/**
 * Main structure for the radio scanner app.
 */
typedef struct {
    Gui* gui;
    uint32_t frequency;
    float rssi;
    SceneManager* scene_manager;
    float sensitivity;
    bool scanning;
    ScanDirection scan_direction;
    Scanner* scanner;
    const SubGhzDevice* radio_device;
    bool speaker_acquired;
    ViewDispatcher* view_dispatcher;
} RadioScannerApp;

void radio_scanner_rx_callback(const void* data, size_t size, void* context);
void radio_scanner_update_rssi(RadioScannerApp* app);
bool radio_scanner_init_subghz(RadioScannerApp* app);
void radio_scanner_process_scanning(RadioScannerApp* app);

void radio_scanner_get_frequency_str(RadioScannerApp* app, FuriString* frequency_str);
void radio_scanner_get_rssi_str(RadioScannerApp* app, FuriString* rssi_str);
void radio_scanner_get_sensitivity_str(RadioScannerApp* app, FuriString* sensitivity_str);
void radio_scanner_get_scanning_str(RadioScannerApp* app, FuriString* scanning_str);
