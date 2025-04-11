#include "../radio_scanner_app_i.h"
#include "../views/scanner.h"

/**
 * Receiver callback for scanner scene events.
 * Sends the received scanner event to the view dispatcher.
 */
void scanner_scene_receiver_callback(ScannerEvent event, void* context) {
    furi_assert(context);
    RadioScannerApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, event);
}

/**
 * Updates the scanner scene by fetching the latest frequency, RSSI, sensitivity,
 * and scanning status, then refreshing the scanner view.
 */
static void scanner_scene_update(void* context) {
    RadioScannerApp* app = context;

    FuriString* frequency_str;
    FuriString* rssi_str;
    FuriString* sensitivity_str;
    FuriString* scanning_str;

    frequency_str = furi_string_alloc();
    rssi_str = furi_string_alloc();
    sensitivity_str = furi_string_alloc();
    scanning_str = furi_string_alloc();

    radio_scanner_get_frequency_str(app, frequency_str);
    radio_scanner_get_rssi_str(app, rssi_str);
    radio_scanner_get_sensitivity_str(app, sensitivity_str);
    radio_scanner_get_scanning_str(app, scanning_str);

    scanner_view_update(
        app->scanner,
        furi_string_get_cstr(frequency_str),
        furi_string_get_cstr(rssi_str),
        furi_string_get_cstr(sensitivity_str),
        furi_string_get_cstr(scanning_str)
    );

    furi_string_free(frequency_str);
    furi_string_free(rssi_str);
    furi_string_free(sensitivity_str);
    furi_string_free(scanning_str);
}

/**
 * Handler called when entering the scanner scene.
 * Sets the scanner view callback and switches the view to the scanner view.
 */
void scanner_scene_on_enter(void* context) {
    RadioScannerApp* app = context;

    scanner_view_set_callback(app->scanner, scanner_scene_receiver_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, RadioScannerViewScanner);
}

/**
 * Handles events for the scanner scene.
 * Processes custom events and tick events.
 */
bool scanner_scene_on_event(void* context, SceneManagerEvent event) {
    UNUSED(event);
    RadioScannerApp* app = context;

    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
            // Scanning
            case ScannerEventScanDirectionDown:
                app->scan_direction = ScanDirectionDown;
                FURI_LOG_I(TAG, "Scan direction set to down");
                consumed = true;
                break;
            case ScannerEventScanDirectionUp:
                app->scan_direction = ScanDirectionUp;
                FURI_LOG_I(TAG, "Scan direction set to up");
                consumed = true;
                break;
            case ScannerEventToggleScanning:
                app->scanning = !app->scanning;
                FURI_LOG_I(TAG, "Toggled scanning: %d", app->scanning);
                consumed = true;
                break;
            // Sensitivity
            case ScannerEventDecreaseSensitivity:
                app->sensitivity -= 1.0f;
                FURI_LOG_I(TAG, "Decreased sensitivity: %f", (double)app->sensitivity);
                consumed = true;
                break;
            case ScannerEventIncreaseSensitivity:
                app->sensitivity += 1.0f;
                FURI_LOG_I(TAG, "Increased sensitivity: %f", (double)app->sensitivity);
                consumed = true;
                break;
            default:
                FURI_LOG_I(TAG, "Unknown event");
                break;
        }
    } else if(event.type == SceneManagerEventTypeTick) {
        if(app->scanning) {
    #ifdef FURI_DEBUG
            FURI_LOG_D(TAG, "Scanning is active");
    #endif
            radio_scanner_process_scanning(app);
        } else {
    #ifdef FURI_DEBUG
            FURI_LOG_D(TAG, "Scanning is inactive, updating RSSI");
    #endif
            radio_scanner_update_rssi(app);
        }

        scanner_scene_update(app);

        consumed = true;
    }

    return consumed;
}

/**
 * Handler called when exiting the scanner scene.
 */
void scanner_scene_on_exit(void* context) {
    UNUSED(context);
}
