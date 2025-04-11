#pragma once

#include "../helpers/scanner_event.h"

#include <gui/view.h>

/**
 * Forward declaration for the Scanner structure.
 * Represents a scanner view with its associated callbacks and context.
 */
typedef struct Scanner Scanner;

/**
 * Function pointer type for scanner event callbacks.
 */
typedef void (*ScannerCallback)(ScannerEvent event, void* context);

/**
 * Structure representing the scanner view.
 */
struct Scanner {
    View* view;
    ScannerCallback callback;
    void* context;
};

/**
 * Data model for the scanner view UI.
 */
typedef struct {
    FuriString* frequency_str;
    FuriString* rssi_str;
    FuriString* sensitivity_str;
    FuriString* scanning_str;
} ScannerModel;

void scanner_view_set_callback(Scanner* scanner, ScannerCallback callback, void* context);

View* scanner_view_get_view(Scanner* scanner);

void scanner_view_update(Scanner* scanner, const char* frequency_str, const char* rssi_str, const char* sensitivity_str, const char* scanning_str);

Scanner* scanner_view_alloc();
void scanner_view_free(Scanner* scanner);
