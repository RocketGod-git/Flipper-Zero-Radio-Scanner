#pragma once

/**
 * Enumeration of scanner events.
 */
typedef enum {
    // Scanning
    ScannerEventScanDirectionDown,
    ScannerEventScanDirectionUp,
    ScannerEventToggleScanning,
    // Views
    ScannerEventViewConfig,
} ScannerEvent;
