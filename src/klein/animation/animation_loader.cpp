#include "klein/animation/animation_loader.hpp"

#include <fstream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "klein/animation/animation.hpp"
#include "klein/vfs/vfs_assets.hpp"

namespace klein::animation {
    LoadedAnimations load_animation_json(const std::filesystem::path &asset) {
        const auto asset_path = vfs::resolve_asset_path(asset);

        spdlog::info("loading animation from \"{}\"", asset_path.string());

        std::ifstream file(asset_path);
        if (!file.is_open())
            throw std::runtime_error("failed to open animation");

        std::vector<AnimationMeta> animations;

        nlohmann::json j;
        file >> j;
        for (const auto& item: j) {
            std::vector<sf::IntRect> frames {};
            for (const auto& frame: item["frames"]) {
                const int x = frame["x"].get<int>();
                const int y = frame["y"].get<int>();
                const int w = frame["w"].get<int>();
                const int h = frame["h"].get<int>();
                frames.emplace_back(sf::IntRect({x, y}, {w, h}));
            }
            animations.push_back({
                .name = item["name"].get<std::string>(),
                .frames = frames,
                .framerate = item["framerate"].get<unsigned int>(),
                .priority = item["priority"].get<int>()
            });
        }

        return LoadedAnimations { animations };
    }
}
