#pragma once

#include <gui/scene_manager.h>

/**
 * Generate enumeration of radio scanner scenes and total number of scenes.
 */
#define ADD_SCENE(name, id) RadioScannerScene##id,
typedef enum {
#include "radio_scanner_scene_config.h"
    RadioScannerSceneNum,
} RadioScannerScene;
#undef ADD_SCENE

/**
 * Scene manager handlers for the radio scanner app.
 */
extern const SceneManagerHandlers radio_scanner_scene_handlers;

/**
 * Generate declarations for scene on_enter handler functions.
 */
#define ADD_SCENE(name, id) void name##_scene_on_enter(void*);
#include "radio_scanner_scene_config.h"
#undef ADD_SCENE

/**
 * Generate declarations for scene on_event handler functions.
 */
#define ADD_SCENE(name, id) \
    bool name##_scene_on_event(void* context, SceneManagerEvent event);
#include "radio_scanner_scene_config.h"
#undef ADD_SCENE

/**
 * Generate declarations for scene on_exit handler functions.
 */
#define ADD_SCENE(name, id) void name##_scene_on_exit(void* context);
#include "radio_scanner_scene_config.h"
#undef ADD_SCENE
