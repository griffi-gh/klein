#include "klein/vfs/vfs_assets.hpp"

#include <format>
#include <spdlog/spdlog.h>

namespace klein::vfs {
    std::filesystem::path asset_path(const std::string asset) {
        auto path_str = std::format(ASSETS_PATH "/{}", asset);
        auto path_normalized = std::filesystem::path(path_str).lexically_normal();
        spdlog::debug("asset path resolved: {} -> {}", asset, path_normalized.string());
        return path_normalized;
    }
}
