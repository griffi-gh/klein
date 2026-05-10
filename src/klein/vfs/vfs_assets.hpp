#pragma once
#include <filesystem>

namespace klein::vfs {
    std::filesystem::path resolve_asset_path(const std::filesystem::path &asset);
}
