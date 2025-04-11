#include "radio_scanner_app_i.h"

#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_gpio.h>
#include <gui/elements.h>
#include <furi_hal_speaker.h>
#include <subghz/devices/devices.h>

/**
 * Custom event callback for the radio scanner app.
 * Passes custom events to the scene manager for handling.
 */
static bool radio_scanner_app_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    RadioScannerApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

/**
 * Back event callback for the radio scanner app.
 * Processes back navigation events via the scene manager.
 */
static bool radio_scanner_app_back_event_callback(void* context) {
    furi_assert(context);
    RadioScannerApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

/**
 * Tick event callback for the radio scanner app.
 * Invokes the scene manager to handle periodic tick events.
 */
static void radio_scanner_app_tick_event_callback(void* context) {
    furi_assert(context);
    RadioScannerApp* app = context;
    scene_manager_handle_tick_event(app->scene_manager);
}

/**
 * Allocates and initializes a new instance of the RadioScannerApp.
 * Sets up GUI components, state variables, and input handlers.
 */
RadioScannerApp* radio_scanner_app_alloc() {
#ifdef FURI_DEBUG
    FURI_LOG_D(TAG, "Enter radio_scanner_app_alloc");
#endif
    RadioScannerApp* app = malloc(sizeof(RadioScannerApp));
    if(!app) {
        FURI_LOG_E(TAG, "Failed to allocate RadioScannerApp");
        return NULL;
    }

    // GUI
    app->gui = furi_record_open(RECORD_GUI);

    // SceneManager
    app->scene_manager = scene_manager_alloc(&radio_scanner_scene_handlers, app);

    // ViewDispatcher
    app->view_dispatcher = view_dispatcher_alloc();

    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(app->view_dispatcher, radio_scanner_app_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(app->view_dispatcher, radio_scanner_app_back_event_callback);
    view_dispatcher_set_tick_event_callback(app->view_dispatcher, radio_scanner_app_tick_event_callback, 100);

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    // Scanner
    app->scanner = scanner_view_alloc();
    view_dispatcher_add_view(app->view_dispatcher, RadioScannerViewScanner, scanner_view_get_view(app->scanner));

    // Init app state
    app->frequency = RADIO_SCANNER_DEFAULT_FREQ;
    app->rssi = RADIO_SCANNER_DEFAULT_RSSI;
    app->sensitivity = RADIO_SCANNER_DEFAULT_SENSITIVITY;
    app->scanning = true;
    app->scan_direction = ScanDirectionUp;
    app->speaker_acquired = false;
    app->radio_device = NULL;

    scene_manager_next_scene(app->scene_manager, RadioScannerSceneScanner);

#ifdef FURI_DEBUG
    FURI_LOG_D(TAG, "Exit radio_scanner_app_alloc");
#endif
    return app;
}

/**
 * Frees all resources allocated by the RadioScannerApp instance.
 * Cleans up GUI, radio device, event queue, and memory.
 */
void radio_scanner_app_free(RadioScannerApp* app) {
    furi_assert(app);
#ifdef FURI_DEBUG
    FURI_LOG_D(TAG, "Enter radio_scanner_app_free");
#endif
    if(app->speaker_acquired && furi_hal_speaker_is_mine()) {
        subghz_devices_set_async_mirror_pin(app->radio_device, NULL);
        furi_hal_speaker_release();
        app->speaker_acquired = false;
#ifdef FURI_DEBUG
        FURI_LOG_D(TAG, "Speaker released");
#endif
    }

    if(app->radio_device) {
        subghz_devices_flush_rx(app->radio_device);
        subghz_devices_stop_async_rx(app->radio_device);
#ifdef FURI_DEBUG
        FURI_LOG_D(TAG, "Asynchronous RX stopped");
#endif
        subghz_devices_idle(app->radio_device);
        subghz_devices_sleep(app->radio_device);
        subghz_devices_end(app->radio_device);
#ifdef FURI_DEBUG
        FURI_LOG_D(TAG, "SubGhzDevice stopped and ended");
#endif
    }

    subghz_devices_deinit();

    // Scanner
    view_dispatcher_remove_view(app->view_dispatcher, RadioScannerViewScanner);
    scanner_view_free(app->scanner);

    // ViewDispatcher
    view_dispatcher_free(app->view_dispatcher);

    // SceneManager
    scene_manager_free(app->scene_manager);

    furi_record_close(RECORD_GUI);

    free(app);
#ifdef FURI_DEBUG
    FURI_LOG_D(TAG, "RadioScannerApp memory freed");
#endif
}

/**
 * Main entry point for the radio scanner app.
 * Handles main loop, input processing, scanning logic, and cleanup.
 */
int32_t radio_scanner_app(void* p) {
    UNUSED(p);
    FURI_LOG_I(TAG, "Enter radio_scanner_app");

    RadioScannerApp* app = radio_scanner_app_alloc();
    if(!app) {
        FURI_LOG_E(TAG, "Failed to allocate app");
        return 1;
    }

    if(!radio_scanner_init_subghz(app)) {
        FURI_LOG_E(TAG, "Failed to initialize SubGHz");
        radio_scanner_app_free(app);
        return 255;
    }
#ifdef FURI_DEBUG
    FURI_LOG_D(TAG, "SubGHz initialized successfully");
#endif

    view_dispatcher_run(app->view_dispatcher);

    radio_scanner_app_free(app);
#ifdef FURI_DEBUG
    FURI_LOG_D(TAG, "Exit radio_scanner_app");
#endif
    return 0;
}
