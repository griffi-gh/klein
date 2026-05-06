#pragma once
#include "entt/entt.hpp"

namespace klein {
    struct DebugState {
        bool show_special = false;
        bool enable_rays = false;
        bool enable_segments = false;
    };

    extern DebugState debug_state;

    void debug_ui();
}
