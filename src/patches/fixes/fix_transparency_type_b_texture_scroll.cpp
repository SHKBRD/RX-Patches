#include "fix_transparency_type_b_texture_scroll.h"

#include "internal/patch.h"
#include "internal/tickable.h"
#include "utils/ppcutil.h"

namespace fix_transparency_type_b_texture_scroll {

TICKABLE_DEFINITION((
        .name = "fix-transparency-type-b-texture-scroll",
        .description = "Transparency Type B texture scroll fix",
        .init_main_loop = init_main_loop, ))

static patch::Tramp<decltype(&mkb::g_something_with_texture_scroll)> s_g_something_with_texture_scroll_tramp;

// g_something_with_texture_scroll updates the scroll matrix, but does not enable
// the texture matrix flag used by sorted draw nodes. Transparency type B models
// use sort_always, so we enable texture scrolling before the original function builds
// the matrix so the sorted draw node accounts for it

void init_main_loop() {
    patch::hook_function(s_g_something_with_texture_scroll_tramp, mkb::g_something_with_texture_scroll, [](mkb::StagedefTextureScroll* tex_scroll) {
        mkb::g_something_with_texture_scroll_2(1);
        s_g_something_with_texture_scroll_tramp.dest(tex_scroll);
    });
}

}// namespace fix_transparency_type_b_texture_scroll
