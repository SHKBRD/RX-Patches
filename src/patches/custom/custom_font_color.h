#pragma once
#include "../mkb/mkb.h"

namespace custom_font_color {
void init_main_loop();
void init_main_game();
void init_sel_ngc();
void tick();

extern mkb::u32 menu_primary_color;
extern mkb::u32 menu_secondary_color;
extern mkb::u32 menu_option_name_color;
extern mkb::u32 current_stage_info_color;
extern mkb::u32 ready_color;
extern mkb::u32 go_color;
extern mkb::u32 fallout_time_over_color;
extern mkb::u32 spin_in_info_color;
extern mkb::u32 final_stage_beginner_color;
extern mkb::u32 final_stage_advanced_color;
extern mkb::u32 final_stage_expert_color;
extern mkb::u32 final_stage_master_color;
extern mkb::u32 bonus_stage_color;


}// namespace custom_font_color