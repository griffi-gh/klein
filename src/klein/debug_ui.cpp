#include "klein/debug_ui.hpp"
#include <imgui.h>

namespace klein {
    DebugState debug_state = {};

    void debug_ui() {
        ImGui::Begin("Debug");
        ImGui::Checkbox("Enable rays", &debug_state.enable_rays);
        ImGui::Checkbox("Enable segments", &debug_state.enable_segments);
        ImGui::End();
    }
}
