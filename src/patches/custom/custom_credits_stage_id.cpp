#include "custom_credits_stage_id.h"

#include "internal/assembly.h"
#include "internal/patch.h"
#include "internal/tickable.h"
#include "utils/ppcutil.h"

namespace custom_credits_stage_id {

int stage_id = 197;

TICKABLE_DEFINITION((
        .name = "custom-credits-stage-id",
        .description = "Custom credits stage ID patch",
        .active_value = stage_id,
        .lower_bound = 0,
        .upper_bound = 420,
        .init_main_game = init_main_game, ))

// In the function which initiates the credits sequence, load our custom stage ID
void init_main_game() {
    stage_id = *active_tickable_ptr->active_value;// Get our stage ID
    patch::write_word(reinterpret_cast<void*>(0x8090A004), PPC_INSTR_LI(PPC_R3, stage_id));
}

}// namespace custom_credits_stage_id
