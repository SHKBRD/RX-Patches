#include "config.h"

#include "internal/assembly.h"
#include "internal/heap.h"
#include "internal/log.h"
#include "internal/tickable.h"
#include "patches/custom/custom_font_color.h"
#include "patches/custom/menu_option_toggle.h"

#define STREQ(x, y) (mkb::strcmp(const_cast<char*>(x), const_cast<char*>(y)) == 0)
#define KEY_ENABLED(x) (STREQ(key, x) && STREQ(value, "enabled"))

namespace config {

static int config_file_length;
static mkb::DVDFileInfo config_file_info;
static char* config_file_buf;
static char config_file_path[] = "/config.txt";

u32 parse_hex_value(char* value) {
    u32 parsed_value = 0;

    // Skip "0x"
    for (char* p = value + 2; *p != '\0'; ++p) {
        u8 current_val;

        if (*p >= '0' && *p <= '9') current_val = *p - '0';
        else if (*p >= 'A' && *p <= 'F') current_val = *p - 'A' + 10;
        else if (*p >= 'a' && *p <= 'f') current_val = *p - 'a' + 10;
        else break;

        parsed_value = (parsed_value << 4) | current_val;
    }

    return parsed_value;
}


u16* parse_stageid_list(char* buf, u16* array) {
    buf = mkb::strchr(buf, '\n') + 1;

    char* end_of_section;
    char key[64] = {0};
    char value[64] = {0};
    end_of_section = mkb::strchr(buf, '}');
    do {
        char *key_start, *key_end, *end_of_line;
        key_start = mkb::strchr(buf, 'E') + 2;
        key_end = mkb::strchr(buf, ':');
        MOD_ASSERT_MSG(key_start < key_end, "Key start after key end, did you start your key with a tab and not spaces?");
        end_of_line = mkb::strchr(buf, '\n');
        mkb::strncpy(key, key_start, (key_end - key_start));
        mkb::strncpy(value, key_end + 2, (end_of_line - key_end) - 2);
        int key_idx = mkb::atoi(key);
        u16 value_short = (u16) mkb::atoi(value);
        array[key_idx] = value_short;

        buf = end_of_line + 1;
        mkb::memset(key, '\0', 64);
        mkb::memset(value, '\0', 64);
    } while (buf < end_of_section);
    return array;
}

void parse_menu_option_toggles(char* buf) {
    buf = mkb::strchr(buf, '\n') + 1;

    char* end_of_section;
    char key[64] = {0};
    char value[64] = {0};
    end_of_section = mkb::strchr(buf, '}');
    do {
        char *key_start, *key_end, *end_of_line;
        key_start = mkb::strchr(buf, '\t') + 1;
        key_end = mkb::strchr(buf, ':');
        MOD_ASSERT_MSG(key_start < key_end, "Key start after key end, did you start your key with a tab and not spaces?");
        end_of_line = mkb::strchr(buf, '\n');
        mkb::strncpy(key, key_start, (key_end - key_start));
        mkb::strncpy(value, key_end + 2, (end_of_line - key_end) - 2);

        if KEY_ENABLED ("monkey-race")
            menu_option_toggle::party_game_bitflag |= 0x1;
        else if KEY_ENABLED ("monkey-fight")
            menu_option_toggle::party_game_bitflag |= 0x2;
        else if KEY_ENABLED ("monkey-target")
            menu_option_toggle::party_game_bitflag |= 0x4;
        else if KEY_ENABLED ("monkey-billiards")
            menu_option_toggle::party_game_bitflag |= 0x8;
        else if KEY_ENABLED ("monkey-bowling")
            menu_option_toggle::party_game_bitflag |= 0x10;
        else if KEY_ENABLED ("monkey-golf")
            menu_option_toggle::party_game_bitflag |= 0x20;
        else if KEY_ENABLED ("monkey-boat")
            menu_option_toggle::party_game_bitflag |= 0x40;
        else if KEY_ENABLED ("monkey-shot")
            menu_option_toggle::party_game_bitflag |= 0x80;
        else if KEY_ENABLED ("monkey-dogfight")
            menu_option_toggle::party_game_bitflag |= 0x100;
        else if KEY_ENABLED ("monkey-soccer")
            menu_option_toggle::party_game_bitflag |= 0x200;
        else if KEY_ENABLED ("monkey-baseball")
            menu_option_toggle::party_game_bitflag |= 0x400;
        else if KEY_ENABLED ("monkey-tennis")
            menu_option_toggle::party_game_bitflag |= 0x800;
        else if KEY_ENABLED ("party-games")
            menu_option_toggle::mode_bitflag &= ~0x2;
        else if KEY_ENABLED ("story-mode")
            menu_option_toggle::main_game_bitflag &= ~0x1;
        else if KEY_ENABLED ("challenge-mode")
            menu_option_toggle::main_game_bitflag &= ~0x2;
        else if KEY_ENABLED ("beginner")
            menu_option_toggle::level_bitflag &= ~0x1;
        else if KEY_ENABLED ("advanced")
            menu_option_toggle::level_bitflag &= ~0x2;
        else if KEY_ENABLED ("expert")
            menu_option_toggle::level_bitflag &= ~0x4;
        else if KEY_ENABLED ("master")
            menu_option_toggle::level_bitflag &= ~0x8;
        buf = end_of_line + 1;
        mkb::memset(key, '\0', 64);
        mkb::memset(value, '\0', 64);
    } while (buf < end_of_section);
}

static void skip_spaces(char*& p) {
    while (*p == ' ' || *p == '\t') p++;
}

static bool parse_csv_ints(char* value, int* out, int count) {
    char* p = value;

    for (int i = 0; i < count; ++i) {
        skip_spaces(p);

        // Parse integer
        out[i] = mkb::atoi(p);

        // Find next comma if more values expected
        if (i + 1 < count) {
            p = mkb::strchr(p, ',');
            if (!p) return false;
            p++;// skip comma
        }
    }

    return true;
}

static bool parse_world_stage_key(const char* key, int& world_idx, int& stage_idx) {
    if (key == nullptr || key[0] != 'W') return false;

    // world starts after 'W'
    int w = mkb::atoi(const_cast<char*>(key + 1));

    // find '-'
    char* dash = mkb::strchr(const_cast<char*>(key), '-');
    if (!dash) return false;

    // stage starts after '-'
    int s = mkb::atoi(dash + 1);

    if (w < 1 || w > main::WORLD_COUNT) return false;
    if (s < 1 || s > main::STAGES_PER_WORLD) return false;

    world_idx = w - 1;
    stage_idx = s - 1;
    return true;
}

static bool parse_csv_lighting(char* value, float* floats, int* hexes) {
    char* p = value;

    for (int i = 0; i < 6; ++i) {
        skip_spaces(p);
        floats[i] = mkb::atof(p);

        p = mkb::strchr(p, ',');
        if (!p) return false;
        p++;
    }

    for (int i = 0; i < 2; ++i) {
        skip_spaces(p);

        MOD_ASSERT_MSG(p[0] == '0' && (p[1] == 'x' || p[1] == 'X'),
                       "Lighting angle values must be hex");

        hexes[i] = parse_hex_value(p);

        if (i + 1 < 2) {
            p = mkb::strchr(p, ',');
            if (!p) return false;
            p++;
        }
    }

    return true;
}

void parse_story_mode_entries(char* buf) {
    buf = mkb::strchr(buf, '\n') + 1;

    char* end_of_section = mkb::strchr(buf, '}');
    char key[64] = {0};
    char value[128] = {0};

    do {
        char *key_start, *key_end, *end_of_line;
        key_start = mkb::strchr(buf, '\t') + 1;
        key_end = mkb::strchr(buf, ':');
        MOD_ASSERT_MSG(key_start < key_end,
                       "Key start after key end, did you start your key with a tab and not spaces?");
        end_of_line = mkb::strchr(buf, '\n');

        mkb::strncpy(key, key_start, (key_end - key_start));
        mkb::strncpy(value, key_end + 2, (end_of_line - key_end) - 2);

        int world_idx = 0, stage_idx = 0;
        MOD_ASSERT_MSG(
            parse_world_stage_key(key, world_idx, stage_idx),
            "Invalid story key format (expected W<1-10>-<1-10>)");

        int vals[3] = {0};

        MOD_ASSERT_MSG(
            parse_csv_ints(value, vals, 3),
            "Invalid story value format (expected: stage ID, time limit, difficulty)");

        int stage_id_i = vals[0];
        int time_limit_i = vals[1];
        int difficulty_i = vals[2];


        MOD_ASSERT_MSG(stage_id_i >= 0 && stage_id_i <= 420, "Stage ID out of range (0-420)");
        MOD_ASSERT_MSG(time_limit_i >= 0 && time_limit_i <= 360, "Time limit out of range (0-360)");
        MOD_ASSERT_MSG(difficulty_i >= 0 && difficulty_i <= 10, "Difficulty out of range (0-10)");

        main::NewStoryStageEntry& entry =
            main::new_story_entries[world_idx][stage_idx];

        MOD_ASSERT_MSG(!entry.set, "Duplicate Story Mode entry for same world/stage");

        entry.stage_id = static_cast<u16>(stage_id_i);
        entry.time_limit = static_cast<u16>(time_limit_i);
        entry.difficulty = static_cast<u8>(difficulty_i);
        entry.set = true;

        buf = end_of_line + 1;
        mkb::memset(key, '\0', sizeof(key));
        mkb::memset(value, '\0', sizeof(value));
    } while (buf < end_of_section);

    for (int w = 0; w < main::WORLD_COUNT; ++w) {
        for (int s = 0; s < main::STAGES_PER_WORLD; ++s) {
            MOD_ASSERT_MSG(main::new_story_entries[w][s].set, "Missing Story Mode entry");
        }
    }

    LOG("Story Mode Entries loaded at: 0x%X", &main::new_story_entries);
}

void parse_world_names(char* buf) {
    buf = mkb::strchr(buf, '\n') + 1;

    char* end_of_section = mkb::strchr(buf, '}');
    char key[64] = {0};
    char value[64] = {0};
    bool seen[main::WORLD_COUNT] = {false};

    do {
        char *key_start, *key_end, *end_of_line;
        key_start = mkb::strchr(buf, '\t') + 1;
        key_end = mkb::strchr(buf, ':');
        MOD_ASSERT_MSG(key_start < key_end,
                       "Key start after key end, did you start your key with a tab and not spaces?");
        end_of_line = mkb::strchr(buf, '\n');

        mkb::strncpy(key, key_start, key_end - key_start);
        mkb::strncpy(value, key_end + 2, (end_of_line - key_end) - 2);

        MOD_ASSERT_MSG(key[0] == 'W', "Invalid world name key format, expected W<1-10>");

        int world_idx = mkb::atoi(key + 1) - 1;

        MOD_ASSERT_MSG(world_idx >= 0 && world_idx < main::WORLD_COUNT,
                       "World name index out of range");
        MOD_ASSERT_MSG(!seen[world_idx],
                       "Duplicate World Name entry");

        mkb::strncpy(
            main::world_names[world_idx],
            value,
            sizeof(main::world_names[world_idx]) - 1);

        main::world_names[world_idx][sizeof(main::world_names[world_idx]) - 1] = '\0';
        seen[world_idx] = true;

        buf = end_of_line + 1;
        mkb::memset(key, '\0', sizeof(key));
        mkb::memset(value, '\0', sizeof(value));
    } while (buf < end_of_section);

    for (int i = 0; i < main::WORLD_COUNT; ++i) {
        MOD_ASSERT_MSG(seen[i], "Missing World Name entry");
    }

    LOG("World Names loaded at: 0x%X", &main::world_names);
}

void parse_font_colors(char* buf) {
    buf = mkb::strchr(buf, '\n') + 1;

    char* end_of_section;
    char key[64] = {0};
    char value[64] = {0};
    end_of_section = mkb::strchr(buf, '}');

    do {
        char *key_start, *key_end, *end_of_line;
        key_start = mkb::strchr(buf, '\t') + 1;
        key_end = mkb::strchr(buf, ':');
        MOD_ASSERT_MSG(key_start < key_end,
                       "Key start after key end, did you start your key with a tab and not spaces?");
        end_of_line = mkb::strchr(buf, '\n');

        mkb::strncpy(key, key_start, (key_end - key_start));
        mkb::strncpy(value, key_end + 2, (end_of_line - key_end) - 2);

        u32 parsed_value = 0;
        if ((value[0] == '0') && (value[1] == 'x' || value[1] == 'X')) {
            parsed_value = parse_hex_value(value);
        }
        else {
            parsed_value = mkb::atoi(value);
        }

        if (STREQ(key, "menu-primary"))
            custom_font_color::menu_primary_color = parsed_value;
        else if (STREQ(key, "menu-secondary"))
            custom_font_color::menu_secondary_color = parsed_value;
        else if (STREQ(key, "menu-option-name"))
            custom_font_color::menu_option_name_color = parsed_value;
        else if (STREQ(key, "current-stage-info"))
            custom_font_color::current_stage_info_color = parsed_value;
        else if (STREQ(key, "ready"))
            custom_font_color::ready_color = parsed_value;
        else if (STREQ(key, "go"))
            custom_font_color::go_color = parsed_value;
        else if (STREQ(key, "fallout-time-over"))
            custom_font_color::fallout_time_over_color = parsed_value;
        else if (STREQ(key, "spin-in-info"))
            custom_font_color::spin_in_info_color = parsed_value;
        else if (STREQ(key, "final-stage-beginner"))
            custom_font_color::final_stage_beginner_color = parsed_value;
        else if (STREQ(key, "final-stage-advanced"))
            custom_font_color::final_stage_advanced_color = parsed_value;
        else if (STREQ(key, "final-stage-expert"))
            custom_font_color::final_stage_expert_color = parsed_value;
        else if (STREQ(key, "final-stage-master"))
            custom_font_color::final_stage_master_color = parsed_value;
        else if (STREQ(key, "bonus-stage"))
            custom_font_color::bonus_stage_color = parsed_value;
        else
            LOG("Unknown font color key %s found in config!", key);

        buf = end_of_line + 1;
        mkb::memset(key, '\0', 64);
        mkb::memset(value, '\0', 64);
    } while (buf < end_of_section);
}

void parse_theme_lights(char* buf) {
    buf = mkb::strchr(buf, '\n') + 1;

    char* end_of_section = mkb::strchr(buf, '}');
    char key[64] = {0};
    char value[128] = {0};

    do {
        char *key_start, *key_end, *end_of_line;
        key_start = mkb::strchr(buf, '\t') + 1;
        key_end = mkb::strchr(buf, ':');
        end_of_line = mkb::strchr(buf, '\n');

        MOD_ASSERT_MSG(key_start < key_end,
                       "Key start after key end, did you start your key with a tab and not spaces?");

        mkb::strncpy(key, key_start, key_end - key_start);
        mkb::strncpy(value, key_end + 2, (end_of_line - key_end) - 2);

        int idx = mkb::atoi(key);

        MOD_ASSERT_MSG(idx >= 0 && idx < main::THEME_LIGHT_COUNT,
                       "Theme light index out of range");

        main::CustomThemeLight& light = main::custom_theme_lights[idx];

        MOD_ASSERT_MSG(!light.set, "Duplicate Theme Light entry");

        float f[6] = {0};
        int h[2] = {0};

        MOD_ASSERT_MSG(
            parse_csv_lighting(value, f, h),
            "Invalid theme light format");

        for (int i = 0; i < 6; ++i) {
            MOD_ASSERT_MSG(f[i] >= 0.0f && f[i] <= 1.0f,
                           "Lighting RGB value out of range, expected 0.00 to 1.00");
        }

        light.light_group_r = f[0];
        light.light_group_g = f[1];
        light.light_group_b = f[2];

        light.light_param_r = f[3];
        light.light_param_g = f[4];
        light.light_param_b = f[5];

        light.xa = static_cast<s16>(h[0]);
        light.ya = static_cast<s16>(h[1]);

        light.set = true;

        buf = end_of_line + 1;
        mkb::memset(key, '\0', sizeof(key));
        mkb::memset(value, '\0', sizeof(value));
    } while (buf < end_of_section);

    LOG("Theme Lights loaded at: 0x%X", &main::custom_theme_lights);
}

void parse_function_toggles(char* buf) {
    // Enters from section parsing, so skip to the next line
    buf = mkb::strchr(buf, '\n') + 1;

    char* end_of_section;
    char key[64] = {0};
    char value[64] = {0};
    end_of_section = mkb::strchr(buf, '}');
    int parsed_value;

    do {
        char *key_start, *key_end, *end_of_line;
        key_start = mkb::strchr(buf, '\t') + 1;
        key_end = mkb::strchr(buf, ':');
        MOD_ASSERT_MSG(key_start < key_end, "Key start after key end, did you start your key with a tab and not spaces?");
        end_of_line = mkb::strchr(buf, '\n');
        mkb::strncpy(key, key_start, (key_end - key_start));
        mkb::strncpy(value, key_end + 2, (end_of_line - key_end) - 2);

        // Set the state of a given tickable based on the found key
        for (const auto& tickable: tickable::get_tickable_manager().get_tickables()) {
            // mkb::OSReport("debug: tickable parse\n");
            if (tickable->name != nullptr && STREQ(key, tickable->name)) {
                // 'value' is enabled, set the value to 1
                if (STREQ(value, "enabled")) {
                    tickable->enabled = true;
                    break;
                }

                // 'value' is disabled, set value to 0
                else if (STREQ(value, "disabled")) {
                    break;
                }

                // 'value' is some integer, set the value and initialize the patch if it differs from the default
                else {
                    // Detect hex (0x or 0X)
                    if ((value[0] == '0') && (value[1] == 'x' || value[1] == 'X')) {
                        parsed_value = parse_hex_value(value);
                    }
                    else {
                        parsed_value = mkb::atoi(value);
                    }

                    // Only set value on tickables that have a defined default active value
                    if (!tickable->active_value.has_value()) break;

                    // Check to see if the passed value is within the defined bounds
                    MOD_ASSERT_MSG(parsed_value >= tickable->lower_bound, "Passed value for patch smaller than minimum value");
                    MOD_ASSERT_MSG(parsed_value <= tickable->upper_bound, "Passed value for patch larger than maximum value");

                    // Set the enabled to the parsed value, if it differs from the default passed value
                    if (parsed_value != tickable->active_value) {
                        tickable->enabled = true;
                        tickable->active_value = parsed_value;
                        break;
                    }

                    // If the value is the default, do not enable the patch
                    else {
                        break;
                    }
                }
            }
        }

        buf = end_of_line + 1;
        mkb::memset(key, '\0', 64);
        mkb::memset(value, '\0', 64);
    } while (buf < end_of_section);
}

void parse_config() {
    bool open_success = mkb::DVDOpen(config_file_path, &config_file_info);
    if (open_success) {
        // heap::alloc rounds to a multiple of 32, necessary for DVDReadAsyncPrio
        config_file_length = (config_file_info.length + 0x1f) & 0xffffffe0;
        config_file_buf = static_cast<char*>(heap::alloc(config_file_length));
        u32 read_length = mkb::read_entire_file_using_dvdread_prio_async(&config_file_info, config_file_buf, config_file_length, 0);
        char* eof = config_file_buf + config_file_info.length;

        if (read_length > 0) {
            LOG("Now parsing config file...");
            char section[64] = {0};
            char* file = config_file_buf;
            do {
                char *section_start, *section_end;
                // Parse the start of a section of the config starting with # and ending with {
                // Example: # Section {
                section_start = mkb::strchr(file, '#');
                section_end = mkb::strchr(file, '{');
                if (section_start != nullptr && section_end != nullptr) {
                    MOD_ASSERT_MSG(section_start < section_end, "Section end before section start, are you sure you started/ended the section segment properly?");
                    // Strip out the '# ' at the start of string, strip out the ' ' at the end
                    section_start += 2;
                    section_end -= 1;

                    mkb::strncpy(section, section_start, (section_end - section_start));

                    LOG("Now parsing category %s...", section);

                    // Parsing function toggles
                    if (STREQ(section, "REL Patches")) {
                        parse_function_toggles(section_end);
                    }

                    else if (STREQ(section, "Menu Option Toggles")) {
                        parse_menu_option_toggles(section_end);
                    }

                    else if (STREQ(section, "Font Colors")) {
                        parse_font_colors(section_end);
                    }

                    else if (STREQ(section, "Theme IDs")) {
                        parse_stageid_list(section_end, main::theme_id_lookup);
                        LOG("Theme ID list loaded at: 0x%X", &main::theme_id_lookup);
                    }

                    else if (STREQ(section, "Difficulty Layout")) {
                        LOG("%s", section);
                    }

                    else if (STREQ(section, "Music IDs")) {
                        parse_stageid_list(section_end, main::bgm_id_lookup);
                        LOG("Music ID list loaded at: 0x%X", &main::bgm_id_lookup);
                    }

                    else if (STREQ(section, "Story Mode Entries")) {
                        parse_story_mode_entries(section_end);
                    }

                    else if (STREQ(section, "World Names")) {
                        parse_world_names(section_end);
                    }

                    else if (STREQ(section, "Theme Lights")) {
                        parse_theme_lights(section_end);
                    }

                    else {
                        LOG("Unknown category %s found in config!", section);
                    }

                    file = mkb::strchr(section_end, '\n') + 1;
                    mkb::memset(section, '\0', 64);
                }
                else {
                    break;
                }
            } while (file <= eof);
        }
        mkb::DVDClose(&config_file_info);
        heap::free(config_file_buf);
    }
}

}// namespace config