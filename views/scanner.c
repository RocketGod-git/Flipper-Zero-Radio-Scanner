#include "scanner.h"
#include "../radio_scanner_app_i.h"

#include <stdio.h>

/**
 * Sets the callback function for scanner view events.
 */
void scanner_view_set_callback(Scanner* scanner, ScannerCallback callback, void* context) {
    furi_assert(scanner);
    furi_assert(callback);
    scanner->callback = callback;
    scanner->context = context;
}

/**
 * Retrieves the view associated with the scanner.
 */
View* scanner_view_get_view(Scanner* scanner) {
    furi_assert(scanner);
    return scanner->view;
}

/**
 * Updates the scanner view with new frequency, RSSI, sensitivity, and scanning status strings.
 */
void scanner_view_update(Scanner* scanner, const char* frequency_str, const char* rssi_str, const char* sensitivity_str, const char* scanning_str) {
    furi_assert(scanner);
    with_view_model(
        scanner->view,
        ScannerModel* model,
        {
            furi_string_set_str(model->frequency_str, frequency_str);
            furi_string_set_str(model->rssi_str, rssi_str);
            furi_string_set_str(model->sensitivity_str, sensitivity_str);
            furi_string_set_str(model->scanning_str, scanning_str);
        },
        true);
}

/**
 * Draw callback for updating the canvas UI.
 * Displays the current frequency, RSSI, sensitivity, and scanning status.
 */
void scanner_view_draw(Canvas* canvas, ScannerModel* model) {
    furi_assert(canvas);
    furi_assert(model);
#ifdef FURI_DEBUG
    FURI_LOG_D(TAG, "Enter scanner_view_draw");
#endif    
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, "Radio Scanner");

    canvas_set_font(canvas, FontSecondary);
    char freq_str[RADIO_SCANNER_BUFFER_SZ + 1] = {0};
    snprintf(freq_str, RADIO_SCANNER_BUFFER_SZ, "Freq: %s MHz", furi_string_get_cstr(model->frequency_str));
    canvas_draw_str_aligned(canvas, 64, 18, AlignCenter, AlignTop, freq_str);

    char rssi_str[RADIO_SCANNER_BUFFER_SZ + 1] = {0};
    snprintf(rssi_str, RADIO_SCANNER_BUFFER_SZ, "RSSI: %s", furi_string_get_cstr(model->rssi_str));
    canvas_draw_str_aligned(canvas, 64, 30, AlignCenter, AlignTop, rssi_str);

    char sensitivity_str[RADIO_SCANNER_BUFFER_SZ + 1] = {0};
    snprintf(sensitivity_str, RADIO_SCANNER_BUFFER_SZ, "Sens: %s", furi_string_get_cstr(model->sensitivity_str));
    canvas_draw_str_aligned(canvas, 64, 42, AlignCenter, AlignTop, sensitivity_str);

    canvas_draw_str_aligned(canvas, 64, 54, AlignCenter, AlignTop, furi_string_get_cstr(model->scanning_str));
#ifdef FURI_DEBUG
    FURI_LOG_D(TAG, "Exit scanner_view_draw");
#endif
}

/**
 * Input callback for handling button events.
 * Passes input events into the event queue for processing.
 */
bool scanner_view_input(InputEvent* event, void* context) {
    furi_assert(context);
    Scanner* scanner = context;
#ifdef FURI_DEBUG
    FURI_LOG_D(TAG, "Enter scanner_view_input");
    FURI_LOG_D(TAG, "Input event: type=%d, key=%d", event->type, event->key);
#endif
    bool consumed = false;
    if(event->type == InputTypeShort) {
        switch(event->key) {
            case InputKeyOk:
                scanner->callback(ScannerEventToggleScanning, scanner->context);
                consumed = true;
                break;
            case InputKeyUp:
                scanner->callback(ScannerEventIncreaseSensitivity, scanner->context);
                consumed = true;
                break;
            case InputKeyDown:
                scanner->callback(ScannerEventDecreaseSensitivity, scanner->context);
                consumed = true;
                break;
            case InputKeyLeft:
                scanner->callback(ScannerEventScanDirectionDown, scanner->context);
                consumed = true;
                break;
            case InputKeyRight:
                scanner->callback(ScannerEventScanDirectionUp, scanner->context);
                consumed = true;
                break;
            default:
                FURI_LOG_I(TAG, "Unknown input");
                break;
        }
    }
    FURI_LOG_D(TAG, "Exit scanner_view_input");
    return consumed;
}

/**
 * Called when entering the scanner view.
 */
void scanner_view_enter(void* context) {
    furi_assert(context);
}

/**
 * Called when exiting the scanner view.
 */
void scanner_view_exit(void* context) {
    furi_assert(context);
}

/**
 * Allocates and initializes a new Scanner instance.
 * Sets up the view, model, and callbacks.
 */
Scanner* scanner_view_alloc() {
    Scanner* scanner = malloc(sizeof(Scanner));

    scanner->view = view_alloc();

    view_allocate_model(scanner->view, ViewModelTypeLocking, sizeof(ScannerModel));
    view_set_context(scanner->view, scanner);
    view_set_draw_callback(scanner->view, (ViewDrawCallback)scanner_view_draw);
    view_set_input_callback(scanner->view, scanner_view_input);
    view_set_enter_callback(scanner->view, scanner_view_enter);
    view_set_exit_callback(scanner->view, scanner_view_exit);

    with_view_model(
        scanner->view,
        ScannerModel* model,
        {
            model->frequency_str = furi_string_alloc();
            model->rssi_str = furi_string_alloc();
            model->sensitivity_str = furi_string_alloc();
            model->scanning_str = furi_string_alloc();
        },
        true
    );

    return scanner;
}

/**
 * Frees the resources associated with the Scanner instance.
 */
void scanner_view_free(Scanner* scanner) {
    furi_assert(scanner);

    with_view_model(
        scanner->view,
        ScannerModel * model,
        {
            furi_string_free(model->frequency_str);
            furi_string_free(model->rssi_str);
            furi_string_free(model->sensitivity_str);
            furi_string_free(model->scanning_str);
        },
        false);

    view_free(scanner->view);

    free(scanner);
}
