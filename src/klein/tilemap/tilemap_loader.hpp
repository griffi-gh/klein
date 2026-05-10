#pragma once

#include <string_view>
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

#include "klein/tilemap/tilemap.hpp"

namespace klein::tilemap {
    TileAttributes parse_tile_attributes(const nlohmann::json &attributes);

    TileMap load_tile_map_data(
        const std::filesystem::path &asset,
        std::string_view name,
        bool compressed = true
    );
}
