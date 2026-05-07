#include "klein/tilemap/tilemap.hpp"
#include <ranges>
#include <SFML/System/Vector2.hpp>
#include <spdlog/spdlog.h>

using std::views::zip, std::views::iota;

namespace klein::tilemap {
    // Update layer lookup table and aabb
    // TODO don't expose this directly, instead do sth to keep tiles consistent
    // mark dirty -> update automatically in getter?
    void TileMapLayer::update_tiles() {
        if (tiles.size() == 0) {
            aabb.position = sf::Vector2i(0, 0);
            aabb.size = sf::Vector2i(0, 0);
            tiles_lut.clear();
            tiles_lut.shrink_to_fit();
            spdlog::debug("layer \"{}\" update_tiles: (empty)", name);
            return;
        }

        // first, compute new aabb
        sf::Vector2i pos_min(INT_MAX, INT_MAX);
        sf::Vector2i pos_max(INT_MIN, INT_MIN);
        for (const auto& tile: tiles) {
            pos_min.x = std::min(pos_min.x, tile.pos.x);
            pos_min.y = std::min(pos_min.y, tile.pos.y);
            pos_max.x = std::max(pos_max.x, tile.pos.x);
            pos_max.y = std::max(pos_max.y, tile.pos.y);
        }
        aabb.position = pos_min;
        aabb.size = pos_max - pos_min + sf::Vector2i(1, 1);

        // update tiles_lut
        tiles_lut.clear();
        tiles_lut.resize(aabb.size.x * aabb.size.y);
        for (const auto& [i, tile]: zip(iota(0uz), tiles)) {
            const size_t lut_idx = (tile.pos.y - pos_min.y) * aabb.size.x + (tile.pos.x - pos_min.x);
            tiles_lut[lut_idx] = i + 1;
        }

        spdlog::debug("layer \"{}\" update_tiles: aabb pos={},{} size={},{}",
            name, aabb.position.x, aabb.position.y, aabb.size.x, aabb.size.y);
    }

    const Tile* TileMapLayer::get(sf::Vector2i tile_coord) const {
        const auto aabb_coord = tile_coord - aabb.position;

        if (
            aabb_coord.x < 0 ||
            aabb_coord.y < 0 ||
            aabb_coord.x >= aabb.size.x ||
            aabb_coord.y >= aabb.size.y
        ) return nullptr;

        const size_t lut_idx = aabb_coord.y * aabb.size.x + aabb_coord.x;
        const size_t tile_idx_plus_one = tiles_lut[lut_idx];
        if (tile_idx_plus_one == 0) return nullptr;

        return &tiles[tile_idx_plus_one - 1];
    }

    const TileMapLayer* TileMap::get_layer_by_name(const std::string &name) const {
        // TODO O(1) lookup
        for (const auto &layer: layers) {
            if (layer.name == name) return &layer;
        }
        return nullptr;
    }
}
