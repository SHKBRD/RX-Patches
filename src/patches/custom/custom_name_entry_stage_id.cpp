#include "custom_name_entry_stage_id.h"

#include "internal/assembly.h"
#include "internal/patch.h"
#include "internal/tickable.h"
#include "utils/ppcutil.h"

namespace custom_name_entry_stage_id {

int stage_id = 200;

TICKABLE_DEFINITION((
        .name = "custom-name-entry-stage-id",
        .description = "Custom name entry stage ID patch",
        .active_value = stage_id,
        .lower_bound = 0,
        .upper_bound = 420,
        .init_main_loop = init_main_loop, ))

// In the function which initiates the name entry sequence, load our custom stage ID
void init_main_loop() {
    stage_id = *active_tickable_ptr->active_value;// Get our stage ID
    patch::write_word(reinterpret_cast<void*>(0x8038ACFC), PPC_INSTR_LI(PPC_R3, stage_id));
}

}// namespace custom_name_entry_stage_id
