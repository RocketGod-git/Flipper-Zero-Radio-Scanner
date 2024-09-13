#include "radio_scanner_app.h"
#include <furi.h>
#include <furi_hal.h>
#include <gui/elements.h>
#include <furi_hal_subghz.h>
#include <furi_hal_speaker.h>

#define TAG "RadioScannerApp"

#define SUBGHZ_FREQUENCY_MIN 300000000
#define SUBGHZ_FREQUENCY_MAX 928000000
#define SUBGHZ_FREQUENCY_STEP 10000

static void radio_scanner_draw_callback(Canvas* canvas, void* context) {
    FURI_LOG_D(TAG, "Entering draw callback");
    RadioScannerApp* app = context;
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, "Radio Scanner");

    canvas_set_font(canvas, FontSecondary);
    char freq_str[32];
    snprintf(freq_str, sizeof(freq_str), "Freq: %.2f MHz", (double)app->frequency / 1000000);
    canvas_draw_str_aligned(canvas, 64, 18, AlignCenter, AlignTop, freq_str);

    char rssi_str[32];
    snprintf(rssi_str, sizeof(rssi_str), "RSSI: %.2f", (double)app->rssi);
    canvas_draw_str_aligned(canvas, 64, 30, AlignCenter, AlignTop, rssi_str);

    char sensitivity_str[32];
    snprintf(sensitivity_str, sizeof(sensitivity_str), "Sens: %.2f", (double)app->sensitivity);
    canvas_draw_str_aligned(canvas, 64, 42, AlignCenter, AlignTop, sensitivity_str);

    canvas_draw_str_aligned(canvas, 64, 54, AlignCenter, AlignTop, app->scanning ? "Scanning..." : "Locked");
    FURI_LOG_D(TAG, "Exiting draw callback");
}

static void radio_scanner_input_callback(InputEvent* input_event, void* context) {
    FURI_LOG_D(TAG, "Entering input callback");
    furi_assert(context);
    FuriMessageQueue* event_queue = context;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
    FURI_LOG_D(TAG, "Exiting input callback");
}

static void radio_scanner_update_rssi(RadioScannerApp* app) {
    FURI_LOG_D(TAG, "Updating RSSI");
    app->rssi = furi_hal_subghz_get_rssi();
    FURI_LOG_D(TAG, "RSSI updated: %.2f", (double)app->rssi);
}

static bool radio_scanner_init_subghz(RadioScannerApp* app) {
    FURI_LOG_D(TAG, "Initializing SubGhz");
    furi_assert(app);
    furi_hal_subghz_reset();
    furi_hal_subghz_idle();
    
    if(!furi_hal_subghz_is_frequency_valid(app->frequency)) {
        FURI_LOG_E(TAG, "Invalid frequency: %lu", app->frequency);
        return false;
    }
    
    FURI_LOG_D(TAG, "Setting frequency: %lu", app->frequency);
    furi_hal_subghz_set_frequency(app->frequency);
    FURI_LOG_D(TAG, "Frequency set");
    furi_hal_subghz_rx();
    FURI_LOG_D(TAG, "SubGhz set to RX mode");
    
    FURI_LOG_D(TAG, "SubGhz initialization complete");
    return true;
}

static void subghz_txrx_rx(RadioScannerApp* app) {
    FURI_LOG_D(TAG, "Entering RX mode");

    furi_hal_subghz_idle();
    furi_hal_subghz_set_frequency(app->frequency);
    furi_hal_subghz_rx();
}

static void subghz_txrx_rx_end(void) {
    FURI_LOG_D(TAG, "Ending RX mode");

    furi_hal_subghz_idle();
}

static bool speaker_on(RadioScannerApp* app) {
    FURI_LOG_D(TAG, "Turning on speaker");
    int retry_count = 0;
    const int max_retries = 5;

    while(retry_count < max_retries) {
        if(furi_hal_speaker_acquire(30)) {
            FURI_LOG_D(TAG, "Speaker acquired");
            if(app->radio_device) {
                subghz_devices_set_async_mirror_pin(app->radio_device, &gpio_speaker);
                FURI_LOG_D(TAG, "Speaker on");
                return true;
            } else {
                FURI_LOG_E(TAG, "Radio device is NULL, cannot set async mirror pin");
            }
        } else {
            FURI_LOG_W(TAG, "Failed to acquire speaker, retrying");
            retry_count++;
            furi_delay_ms(100);
        }
    }

    FURI_LOG_E(TAG, "Failed to acquire speaker after maximum retries");
    return false;
}

static void speaker_off(RadioScannerApp* app) {
    FURI_LOG_D(TAG, "Turning off speaker");
    if(furi_hal_speaker_is_mine()) {
        if(app->radio_device) {
            subghz_devices_set_async_mirror_pin(app->radio_device, NULL);
            FURI_LOG_D(TAG, "Stopped routing RF signal to speaker");
        }
        furi_hal_speaker_release();
        FURI_LOG_D(TAG, "Speaker off");
    }
}

static void radio_scanner_process_scanning(RadioScannerApp* app) {
    FURI_LOG_D(TAG, "Processing scanning");
    radio_scanner_update_rssi(app);

    bool signal_detected = (app->rssi > app->sensitivity);
    
    if(signal_detected) {
        FURI_LOG_I(TAG, "Signal detected above sensitivity threshold");
        if(app->scanning) {
            FURI_LOG_D(TAG, "Locking frequency due to signal detection");
            speaker_on(app);  
            app->scanning = false;  
        }
    } else {
        FURI_LOG_D(TAG, "No signal detected, continue scanning");
        app->scanning = true;  
    }

    if(app->scanning) {
        uint32_t new_frequency;
        if(app->scan_direction == ScanDirectionUp) {
            new_frequency = app->frequency + SUBGHZ_FREQUENCY_STEP;
        } else {
            new_frequency = app->frequency - SUBGHZ_FREQUENCY_STEP;
        }

        FURI_LOG_D(TAG, "Calculated new frequency: %lu", new_frequency);

        if(new_frequency > SUBGHZ_FREQUENCY_MAX || new_frequency < SUBGHZ_FREQUENCY_MIN) {
            new_frequency = SUBGHZ_FREQUENCY_MIN;
            FURI_LOG_D(TAG, "Frequency reset to minimum: %lu", new_frequency);
        }

        if(furi_hal_subghz_is_frequency_valid(new_frequency)) {
            FURI_LOG_D(TAG, "Setting new frequency: %lu", new_frequency);
            subghz_txrx_rx_end();  
            app->frequency = new_frequency;
            subghz_txrx_rx(app); 
        } else {
            FURI_LOG_W(TAG, "Invalid frequency: %lu, skipping", new_frequency);
            app->frequency = SUBGHZ_FREQUENCY_MIN;
        }
    }

    FURI_LOG_D(TAG, "Scanning process complete");
}

RadioScannerApp* radio_scanner_app_alloc() {
    FURI_LOG_D(TAG, "Allocating RadioScannerApp");
    RadioScannerApp* app = malloc(sizeof(RadioScannerApp));
    app->view_port = view_port_alloc();
    app->event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    app->radio_device = NULL;
    app->running = true;
    app->frequency = 433920000;
    app->rssi = -100.0f;
    app->sensitivity = -105.0f;
    app->scanning = true;
    app->scan_direction = ScanDirectionUp;
    view_port_draw_callback_set(app->view_port, radio_scanner_draw_callback, app);
    view_port_input_callback_set(app->view_port, radio_scanner_input_callback, app->event_queue);
    FURI_LOG_D(TAG, "RadioScannerApp allocated");
    return app;
}

void radio_scanner_app_free(RadioScannerApp* app) {
    FURI_LOG_D(TAG, "Freeing RadioScannerApp");
    furi_assert(app);
    view_port_free(app->view_port);
    furi_message_queue_free(app->event_queue);
    free(app);
    FURI_LOG_D(TAG, "RadioScannerApp freed");
}

int32_t radio_scanner_app(void* p) {
    UNUSED(p);
    FURI_LOG_D(TAG, "Starting RadioScannerApp");
    RadioScannerApp* app = radio_scanner_app_alloc();

    FURI_LOG_D(TAG, "Opening GUI");
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, app->view_port, GuiLayerFullscreen);

    FURI_LOG_D(TAG, "Initializing SubGhz");
    if(!radio_scanner_init_subghz(app)) {
        FURI_LOG_E(TAG, "Failed to initialize SubGhz");
        radio_scanner_app_free(app);
        return 255;
    }

    FURI_LOG_D(TAG, "Entering main loop");
    InputEvent event;
    while(app->running) {
        if(app->scanning) {
            radio_scanner_process_scanning(app);
        } else {
            radio_scanner_update_rssi(app);
        }

        if(furi_message_queue_get(app->event_queue, &event, 10) == FuriStatusOk) {
            FURI_LOG_D(TAG, "Input event received: %d", event.key);
            if(event.type == InputTypeShort) {
                if(event.key == InputKeyOk) {
                    FURI_LOG_D(TAG, "OK button pressed");
                    app->scanning = !app->scanning;
                } else if(event.key == InputKeyUp) {
                    FURI_LOG_D(TAG, "Up button pressed, increasing sensitivity");
                    app->sensitivity += 5.0f;
                    FURI_LOG_D(TAG, "New sensitivity: %.2f", (double)app->sensitivity);
                } else if(event.key == InputKeyDown) {
                    FURI_LOG_D(TAG, "Down button pressed, decreasing sensitivity");
                    app->sensitivity -= 5.0f;
                    FURI_LOG_D(TAG, "New sensitivity: %.2f", (double)app->sensitivity);
                } else if(event.key == InputKeyLeft) {
                    FURI_LOG_D(TAG, "Left button pressed, changing scan direction to down");
                    app->scan_direction = ScanDirectionDown;
                } else if(event.key == InputKeyRight) {
                    FURI_LOG_D(TAG, "Right button pressed, changing scan direction to up");
                    app->scan_direction = ScanDirectionUp;
                } else if(event.key == InputKeyBack) {
                    FURI_LOG_D(TAG, "Back button pressed, exiting app");
                    app->running = false;
                }
            }
        }

        view_port_update(app->view_port);
        furi_delay_ms(10);
    }

    FURI_LOG_D(TAG, "Exiting main loop");
    furi_hal_subghz_idle();
    furi_hal_subghz_sleep();
    speaker_off(app);

    FURI_LOG_D(TAG, "Removing view port");
    gui_remove_view_port(gui, app->view_port);
    furi_record_close(RECORD_GUI);

    FURI_LOG_D(TAG, "Freeing app resources");
    radio_scanner_app_free(app);
    FURI_LOG_I(TAG, "RadioScannerApp finished");
    return 0;
}
