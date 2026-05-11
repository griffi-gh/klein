#pragma once
#include <filesystem>
#include <vector>
#include "klein/animation/animation.hpp"

namespace klein::animation {
    struct LoadedAnimations {
        std::vector<AnimationMeta> animations;
    };

    LoadedAnimations load_animation_json(const std::filesystem::path &asset);
}
