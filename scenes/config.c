#include "../radio_scanner_app_i.h"

/**
 * Enumeration of configuration indices used for accessing specific settings.
 */
enum ConfigIndex {
    ConfigIndexSound,
    ConfigIndexSensitivity,
};

// Text labels for the sound state values.
const char* const sound_state_text[SoundStateNum] = {
    "OFF",
    "SQLCH",
    "ON",
};

// Corresponding enum/int values for sound states.
const uint32_t sound_state_value[SoundStateNum] = {
    SoundStateOFF,
    SoundStateSquelch,
    SoundStateON,
};

/**
 * Callback to update the sound state when changed in the config UI.
 */
static void config_scene_set_sound_state(VariableItem* item) {
    RadioScannerApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    
    variable_item_set_current_value_text(item, sound_state_text[index]);

    app->sound_state = sound_state_value[index];

    if(app->speaker_acquired) {
        // Stop RX before changing the mirror pin
        subghz_devices_stop_async_rx(app->radio_device);
        // Update the mirror pin based on the sound state
        if(app->sound_state == SoundStateON) {
            subghz_devices_set_async_mirror_pin(app->radio_device, &gpio_speaker);
        } else {
            subghz_devices_set_async_mirror_pin(app->radio_device, NULL);
        }
        // Restart RX
        subghz_devices_start_async_rx(app->radio_device, radio_scanner_rx_callback, app);
    }
}

/**
 * Utility function to get the index of a sound state value.
 */
uint8_t config_scene_sound_value_index(const uint32_t value, const uint32_t values[], uint8_t values_count, void* context) {
    furi_assert(context);
    UNUSED(context);

    for(uint8_t i = 0; i < values_count; i++){
        if(values[i] == value) {
            return i;
        }
    }

    return 0;
}

/**
 * Helper function to update the sensitivity display text.
 */
static void config_scene_set_sensitivity_display_text(VariableItem* item, float sensitivity) {
    char buf[8];
    snprintf(buf, sizeof(buf), "%.1f", (double)sensitivity);
    variable_item_set_current_value_text(item, buf);
}

/**
 * Helper function to compute the configuration index based on the current sensitivity value.
 *
 * Sensitivity is a float where 0.0 corresponds to index 0 and -127.0 corresponds to index 127.
 * This function clamps the value to the allowed range and returns the proper index.
 */
static uint8_t config_scene_sensitivity_value_index(float sensitivity) {
    if(sensitivity > 0.0f) {
        sensitivity = 0.0f;
    } else if(sensitivity < -127.0f) {
        sensitivity = -127.0f;
    }
    return (uint8_t)(sensitivity + 127.0f);
}

/**
 * Callback to update the sensitivity value when changed in the config UI.
 */
static void config_scene_set_sensitivity(VariableItem* item) {
    RadioScannerApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    // Calculate sensitivity: index 0 = -127.0, index 1 = -126.0, ..., index 127 = 0.0
    float sensitivity = -127.0f + (float)index;
    app->sensitivity = sensitivity;

    config_scene_set_sensitivity_display_text(item, sensitivity);
}

/**
 * Handler called when entering the config scene.
 * Sets the config view callback and switches the view to the config view.
 */
void config_scene_on_enter(void* context) {
    RadioScannerApp* app = context;
    VariableItem* item;
    uint8_t value_index;

    // Sound
    item = variable_item_list_add(
        app->config,
        "Sound:",
        SoundStateNum,
        config_scene_set_sound_state,
        app
    );
    value_index = config_scene_sound_value_index(app->sound_state, sound_state_value, SoundStateNum, app);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, sound_state_text[value_index]);

    // Sensitivity
    item = variable_item_list_add(
        app->config,
        "Sensitivity:",
        128,
        config_scene_set_sensitivity,
        app
    );
    uint8_t sensitivity_index = config_scene_sensitivity_value_index(app->sensitivity);
    variable_item_set_current_value_index(item, sensitivity_index);
    config_scene_set_sensitivity_display_text(item, app->sensitivity);

    view_dispatcher_switch_to_view(app->view_dispatcher, RadioScannerViewConfig);
}

/**
 * Handles events for the config scene.
 * Processes custom events.
 */
bool config_scene_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

/**
 * Handler called when exiting the config scene.
 */
void config_scene_on_exit(void* context) {
    RadioScannerApp* app = context;
    variable_item_list_set_selected_item(app->config, 0);
    variable_item_list_reset(app->config);
}
