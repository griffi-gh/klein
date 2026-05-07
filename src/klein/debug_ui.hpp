#pragma once
#include <entt/entt.hpp>

namespace klein {
    struct DebugState {
        bool show_special = false;
        bool enable_rays = false;
        bool enable_segments = false;
        bool debug_tile_composer = false;
    };

    extern DebugState debug_state;

    void debug_ui();
}
