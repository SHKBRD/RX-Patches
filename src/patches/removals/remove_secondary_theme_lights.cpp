#include "remove_secondary_theme_lights.h"

#include "internal/patch.h"
#include "internal/tickable.h"
#include "utils/ppcutil.h"

namespace remove_secondary_theme_lights {

TICKABLE_DEFINITION((
        .name = "remove-secondary-theme-lights",
        .description = "Remove secondary theme lights",
        .init_main_loop = init_main_loop, ))

void init_main_loop() {
    // In arrays which check for hardcoded light theme IDs and settings,
    // check for themes 0xffffffff instead of Volcanic Magma and Dr.
    // BAD-BOON's Base
    patch::write_word(reinterpret_cast<void*>(0x804551B8), 0xffffffff);
    patch::write_word(reinterpret_cast<void*>(0x80455208), 0xffffffff);
    // Overwrite Dr. BAD-BOON's Base's display/tick functions with
    // the defaults
    patch::write_word(reinterpret_cast<void*>(0x804749C0), 0x802E11BC);
    patch::write_word(reinterpret_cast<void*>(0x80474868), 0x802E111C);
}

}// namespace remove_secondary_theme_lights
