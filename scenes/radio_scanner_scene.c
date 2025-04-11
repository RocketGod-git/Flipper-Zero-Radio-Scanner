#include "../radio_scanner_app_i.h"

/**
 * Generate array of on_enter handler functions for each scene.
 */
#define ADD_SCENE(name, id) name##_scene_on_enter,
void (*const radio_scanner_scene_on_enter_handlers[])(void*) = {
#include "radio_scanner_scene_config.h"
};
#undef ADD_SCENE

/**
 * Generate array of on_event handler functions for each scene.
 */
#define ADD_SCENE(name, id) name##_scene_on_event,
bool (*const radio_scanner_scene_on_event_handlers[])(void* context, SceneManagerEvent event) = {
#include "radio_scanner_scene_config.h"
};
#undef ADD_SCENE

/**
 * Generate array of on_exit handler functions for each scene.
 */
#define ADD_SCENE(name, id) name##_scene_on_exit,
void (*const radio_scanner_scene_on_exit_handlers[])(void* context) = {
#include "radio_scanner_scene_config.h"
};
#undef ADD_SCENE

/**
 * Scene manager handlers configuration structure for the radio scanner app.
 */
const SceneManagerHandlers radio_scanner_scene_handlers = {
    .on_enter_handlers = radio_scanner_scene_on_enter_handlers,
    .on_event_handlers = radio_scanner_scene_on_event_handlers,
    .on_exit_handlers = radio_scanner_scene_on_exit_handlers,
    .scene_num = RadioScannerSceneNum,
};
