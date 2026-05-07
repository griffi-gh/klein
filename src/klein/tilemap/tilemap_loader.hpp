#pragma once

#include <string>
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

#include "klein/tilemap/tilemap.hpp"

namespace klein::tilemap {
    TileAttributes parse_tile_attributes(const nlohmann::json &attributes);

    TileMap load_tile_map_data(std::string asset, std::string name, bool compressed = true);

    // TileMap load_
}
