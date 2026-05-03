#pragma once
#include <filesystem>
#include <string>

namespace klein::vfs {
    std::filesystem::path asset_path(const std::string asset);
}
