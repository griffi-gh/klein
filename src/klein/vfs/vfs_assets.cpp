#include "klein/vfs/vfs_assets.hpp"

#include <filesystem>
#include <spdlog/spdlog.h>

namespace klein::vfs {
    std::filesystem::path resolve_asset_path(const std::filesystem::path& asset) {
        std::filesystem::path assets_path = ASSETS_PATH;
        if (assets_path.is_relative())
            assets_path = std::filesystem::absolute(assets_path);
        auto resolved = (assets_path / asset).lexically_normal();
        spdlog::debug("asset path resolved: {} -> {}",
            asset.string(), resolved.string());
        return resolved;
    }
}
