#pragma once

/**
 * Enumeration of scanner events.
 */
typedef enum {
    // Scanning
    ScannerEventScanDirectionDown,
    ScannerEventScanDirectionUp,
    ScannerEventToggleScanning,
    // Sensitivity
    ScannerEventDecreaseSensitivity,
    ScannerEventIncreaseSensitivity,
} ScannerEvent;
