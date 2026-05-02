#pragma once
#include <filesystem>
#include <string>

namespace klein::assets {
    std::filesystem::path resolve_path(const std::string asset);
}
