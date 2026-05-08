#pragma once

namespace klein::debug {
    struct DebugFlags {
        bool show_special = false;
        bool enable_rays = false;
        bool enable_segments = false;
        bool debug_tile_composer = false;
    };

    extern DebugFlags flags;
}
