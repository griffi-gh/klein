#include "klein/debug/debug_ui.hpp"

#include <array> // IWYU pragma: keep
#include <vector> // IWYU pragma: keep
#include <deque>
#include <SFML/System/Time.hpp>
#include <SFML/System/Clock.hpp>
#include <imgui.h>

#include "klein/debug/debug.hpp"

namespace klein::debug {
    void debug_ui() {
        ImGui::Begin("Debug", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        static sf::Clock clock {};
        static std::deque<float> samples {};
        constexpr size_t WINDOW_SIZE = 600;

        float dt = clock.restart().asSeconds();
        samples.push_back(dt);
        if (samples.size() > WINDOW_SIZE) samples.pop_front();
        float dt_avg = 0.f;
        for (float f: samples) dt_avg += f;
        dt_avg /= samples.size();

        ImGui::SeparatorText("Performance");

        static std::array<float, WINDOW_SIZE> temp {};
        // copy to array
        std::copy(samples.begin(), samples.end(), temp.begin());
        std::string frametime_text = std::format("AVG {:.5f}ms ({:.01f} FPS)", dt_avg, 1. / dt_avg);
        ImGui::PlotLines("Frametime", temp.data(), temp.size(), 0, frametime_text.c_str(), 0.0f, 1 / 60.0f, ImVec2(0, 80));

        ImGui::SeparatorText("Flags");

        ImGui::Checkbox("Show Special layer", &flags.show_special);
        ImGui::Checkbox("Enable rays", &flags.enable_rays);
        ImGui::Checkbox("Enable segments", &flags.enable_segments);
        ImGui::Checkbox("Enable debug tile composer", &flags.debug_tile_composer);
        ImGui::Checkbox("Purge/disable tile composer cache (slow!)", &flags.tile_composer_purge);
        ImGui::Checkbox("Show player hitbox", &flags.player_hitbox);

        ImGui::End();
    }
}
