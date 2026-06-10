#include "custom_theme_lights.h"

#include "internal/assembly.h"
#include "internal/patch.h"
#include "internal/tickable.h"
#include "utils/ppcutil.h"

namespace custom_theme_lights {

TICKABLE_DEFINITION((
        .name = "custom-theme-lights",
        .description = "Custom lighting patch",
        .init_main_loop = init_main_loop, ))

void init_main_loop() {
    for (int i = 0; i < main::THEME_LIGHT_COUNT; ++i) {
        main::CustomThemeLight& custom = main::custom_theme_lights[i];

        if (!custom.set) {
            continue;
        }

        mkb::theme_lights[i].light_group_r = custom.light_group_r;
        mkb::theme_lights[i].light_group_g = custom.light_group_g;
        mkb::theme_lights[i].light_group_b = custom.light_group_b;

        mkb::theme_lights[i].light_param_r = custom.light_param_r;
        mkb::theme_lights[i].light_param_g = custom.light_param_g;
        mkb::theme_lights[i].light_param_b = custom.light_param_b;

        mkb::theme_lights[i].xa = custom.xa;
        mkb::theme_lights[i].ya = custom.ya;
    }
}

}// namespace custom_theme_lights
