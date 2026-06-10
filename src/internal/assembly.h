#pragma once

#include "mkb/mkb.h"

namespace main {

struct NewStoryStageEntry {
    u16 stage_id;  // 0 to 420
    u8 difficulty; // 0 to 10
    u16 time_limit;// 0 to 360 seconds
    bool set;      // to detect missing/duplicate entries
};

static constexpr int WORLD_COUNT = 10;
static constexpr int STAGES_PER_WORLD = 10;

extern NewStoryStageEntry new_story_entries[WORLD_COUNT][STAGES_PER_WORLD];

extern char world_names[WORLD_COUNT][64];

static constexpr int THEME_LIGHT_COUNT = 42;

struct CustomThemeLight {
    float light_group_r;
    float light_group_g;
    float light_group_b;

    float light_param_r;
    float light_param_g;
    float light_param_b;

    s16 xa;
    s16 ya;

    bool set;
};

extern CustomThemeLight custom_theme_lights[THEME_LIGHT_COUNT];

extern "C" {

// Assembly overwrite functions
void full_debug_text_color();

// main.cpp
void start_main_loop_assembly();

// Functions accessed by assembly overwrites
// main.cpp
void tick();

// Full color to draw debug text in, if not zero
// Normally, the debug text drawing function uses a single byte for the color in the form RRGGBBAA
extern mkb::GXColor debug_text_color;

extern bool debug_mode_enabled;

// story_mode_music_fix
void story_mode_music_hook();

// stobj_reflection_fix
void reflection_draw_stage_hook();
void reflection_view_stage_hook();

// music_id_per_stage
extern u16 bgm_id_lookup[421];// TODO: make dynamic
void get_bgm_id_hook();

// theme_id_per_stage
extern u16 theme_id_lookup[421];// TODO: make dynamic
void get_theme_id_hook_1();
void get_theme_id_hook_2();

// story_mode_char_select
void get_monkey_id_hook();
}

}// namespace main
