#include "custom_font_color.h"

#include "../internal/patch.h"
#include "../internal/tickable.h"
#include "../mkb/mkb.h"
#include "../utils/ppcutil.h"

namespace custom_font_color {

TICKABLE_DEFINITION((
        .name = "custom-font-colors",
        .description = "Custom font colors",
        .init_main_loop = init_main_loop,
        .init_main_game = init_main_game,
        .init_sel_ngc = init_sel_ngc,
        .tick = tick, ))


mkb::u32 menu_primary_color = 0xFFC000;
mkb::u32 menu_secondary_color = 0xFFFF00;
mkb::u32 menu_option_name_color = 0x0000C0;
mkb::u32 current_stage_info_color = 0xFFC000;
mkb::u32 ready_color = 0xFFC800;
mkb::u32 go_color = 0x0080FF;
mkb::u32 fallout_time_over_color = 0xFF8C00;
mkb::u32 spin_in_info_color = 0xFFFF00;
mkb::u32 final_stage_beginner_color = 0x00D000;
mkb::u32 final_stage_advanced_color = 0x0000E0;
mkb::u32 final_stage_expert_color = 0xFFC000;
mkb::u32 final_stage_master_color = 0xFFC000;
mkb::u32 bonus_stage_color = 0xFF8000;

static patch::Tramp<decltype(&mkb::textdraw_set_mul_color)> s_textdraw_set_mul_color_tramp;

void replace_lis_color(int reg, u32 loc, u32 color) {
    patch::write_word(reinterpret_cast<void*>(loc), PPC_INSTR_LIS(reg, (color >> 16) & 0xff));// RR
    patch::write_word(reinterpret_cast<void*>(loc + 4), PPC_INSTR_ORI(reg, (color & 0xffff)));// GGBB
}

void replace_3pt_color(u32 loc, u32 color) {
    patch::write_word(reinterpret_cast<void*>(loc), 0x38000000 + ((color >> 16) & 0xff));   // RR
    patch::write_word(reinterpret_cast<void*>(loc + 8), 0x38000000 + ((color >> 8) & 0xff));// GG
    patch::write_word(reinterpret_cast<void*>(loc + 16), 0x38000000 + (color & 0xff));      // BB
}

u32 form_color(u8 red, u8 green, u8 blue) {
    return (red << 16) + (green << 8) + blue;
}

u32 convert_color(u32 color) {
    u8 red = (color >> 16) & 0xff;
    u8 green = (color >> 8) & 0xff;
    u8 blue = color & 0xff;
    u32 new_color = color;

    if (blue == 0 && red > 0 && green > 0 && red >= green) {// Orange & yellow (b=0, r>=g>0)
        u8 red_1 = (menu_primary_color >> 16) & 0xff;
        u8 green_1 = (menu_primary_color >> 8) & 0xff;
        u8 blue_1 = menu_primary_color & 0xff;

        u8 red_2 = (menu_secondary_color >> 16) & 0xff;
        u8 green_2 = (menu_secondary_color >> 8) & 0xff;
        u8 blue_2 = menu_secondary_color & 0xff;

        float percent_primary = (1 - ((float) (red - green) / (red / 2)));

        new_color = form_color(
            (u8) (((float) red / 0xff) * ((red_1 * percent_primary) + (red_2 * (1 - percent_primary)))),
            (u8) (((float) red / 0xff) * ((green_1 * percent_primary) + (green_2 * (1 - percent_primary)))),
            (u8) (((float) red / 0xff) * ((blue_1 * percent_primary) + (blue_2 * (1 - percent_primary)))));
    }

    return new_color;
}


static patch::Tramp<decltype(&mkb::parse_avtext_color_codes)> s_parse_avtext_color_codes_tramp;

mkb::u32 parse_avtext_color_codes_hook(char* string, mkb::SpriteDrawRequest* sprite_draw_req) {
    mkb::u32 ret = s_parse_avtext_color_codes_tramp.dest(string, sprite_draw_req);

    if (sprite_draw_req != nullptr && ret != 0) {
        if (sprite_draw_req->mult_color == 0xFF8000) {
            sprite_draw_req->mult_color = menu_secondary_color;
        }
        else if (sprite_draw_req->mult_color == 0xFFFF00) {
            sprite_draw_req->mult_color = menu_primary_color;
        }
    }

    return ret;
}

void set_menu_title_color() {
    mkb::textdraw_set_mul_color(menu_option_name_color);
}
void set_some_stagename_colors() {
    mkb::textdraw_set_mul_color(current_stage_info_color);
}
void init_main_loop() {
    patch::hook_function(s_textdraw_set_mul_color_tramp, mkb::textdraw_set_mul_color, [](u32 color) {
        if (mkb::textdraw_font == mkb::FONT_ASC_24x24) {
            s_textdraw_set_mul_color_tramp.dest(convert_color(color));
        }
        else {
            s_textdraw_set_mul_color_tramp.dest(color);
        }
    });
    patch::hook_function(
        s_parse_avtext_color_codes_tramp,
        mkb::parse_avtext_color_codes,
        parse_avtext_color_codes_hook);
    patch::write_branch_bl(
        reinterpret_cast<void*>(0x8032C588),
        reinterpret_cast<void*>(set_some_stagename_colors));

    replace_3pt_color(0x80338c84, current_stage_info_color);  // Stage number RGB color
    replace_3pt_color(0x80338e74, current_stage_info_color);  // Stage name RGB color
    replace_3pt_color(0x80338A84, current_stage_info_color);  // EX color
    replace_3pt_color(0x8032caa0, ready_color);               // READY text color
    replace_3pt_color(0x8032d944, go_color);                  // GO text color
    replace_3pt_color(0x8032bbb8, spin_in_info_color);        // STAGE/WORLD spin in color
    replace_3pt_color(0x8032E91C, fallout_time_over_color);   // FALLOUT color
    replace_3pt_color(0x8032ED18, fallout_time_over_color);   // TIME OVER color
    replace_3pt_color(0x8032F01C, fallout_time_over_color);   // BONUS FINISH color
    replace_3pt_color(0x8032CF20, final_stage_beginner_color);// Beginner FINAL STAGE color
    replace_3pt_color(0x8032CF50, final_stage_advanced_color);// Advanced FINAL STAGE color
    replace_3pt_color(0x8032BE70, bonus_stage_color);         // BONUS STAGE color
}

void init_main_game() {
    patch::write_branch_bl(
        reinterpret_cast<void*>(0x80900DF8),
        reinterpret_cast<void*>(set_some_stagename_colors));
}

void init_sel_ngc() {
    patch::write_branch_bl(
        reinterpret_cast<void*>(0x8090C5F0),
        reinterpret_cast<void*>(set_menu_title_color));
    patch::write_branch_bl(
        reinterpret_cast<void*>(0x8090C7E8),
        reinterpret_cast<void*>(set_menu_title_color));
}

void tick() {
    if (((mkb::main_game_mode == mkb::PRACTICE_MODE) &&
         ((mkb::g_mode_flags2 & (mkb::MF_PLAYING_MASTER_EX_COURSE |
                                 mkb::MF_PLAYING_MASTER_NOEX_COURSE)) != mkb::MF_NONE)) ||
        ((mkb::mode_flags & (mkb::MF_PLAYING_MASTER_EX_COURSE |
                             mkb::MF_PLAYING_MASTER_NOEX_COURSE)) != mkb::MF_NONE)) {
        replace_3pt_color(0x8032CF6C, final_stage_master_color);
    }
    else {
        replace_3pt_color(0x8032CF6C, final_stage_expert_color);
    }
}

}// namespace custom_font_color