#include "assembly.h"

namespace main {

// TODO: make dyanmic
u16 bgm_id_lookup[421] = {0};
u16 theme_id_lookup[421] = {0};
NewStoryStageEntry new_story_entries[WORLD_COUNT][STAGES_PER_WORLD];

char world_names[WORLD_COUNT][64] = {
    "Jungle Island",
    "Volcanic Magma",
    "Under the Ocean",
    "Inside a Whale",
    "Amusement Park",
    "Boiling Pot",
    "Bubbly Washing Machine",
    "Clock Tower Factory",
    "Space Colony",
    "Dr. BAD-BOON's Base",
};

CustomThemeLight custom_theme_lights[THEME_LIGHT_COUNT];

}// namespace main
