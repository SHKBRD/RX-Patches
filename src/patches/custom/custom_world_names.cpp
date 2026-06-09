#include "custom_world_names.h"

#include "internal/assembly.h"
#include "internal/patch.h"
#include "internal/tickable.h"
#include "utils/ppcutil.h"

namespace custom_world_names {

TICKABLE_DEFINITION((
        .name = "custom-world-names",
        .description = "Custom world names",
        .init_main_game = init_main_game, ))


void init_main_game() {
    for (int i = 0; i < main::WORLD_COUNT; ++i) {
        mkb::world_names[i * 6] = main::world_names[i];
    }
}
}// namespace custom_world_names
