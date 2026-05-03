#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "SFML/Graphics/Rect.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/System/Vector2.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace klein::tilemap {
    class Spritesheet {
    public:
        sf::Texture texture;
        sf::Vector2u tile_size;
    };

    struct Tile {
        sf::Vector2i pos; /**< Global position (NOT offset by layer's aabb_origin */
        uint16_t tex_id;
        std::optional<json> attributes = std::nullopt;
    };

    class TileMapLayer {
    public: // TODO: these should be private (accessible through a stable interface)
        std::string name = "unnamed";

        std::vector<Tile> tiles;

        /// Part of the tilemap covered by this layer, in tiles
        ///
        sf::IntRect aabb;

        /// Set to true when the layer is considered a collider.
        /// Tiles from this layer will block player movement/collide with them
        ///
        bool collider = false;

        /// Call after mutating tiles
        ///
        void update_tiles();

        /// Get tile at specified coord (world-space)
        ///
        const Tile* get(sf::Vector2i tile_coord) const;

    private:
        /// Maps maps Pos -> Index in `tiles`
        //
        /// - Keys are layer-relative! (see `aabb`)\
        /// - Values are shifted by 1 (0 is sentinel for empty cell)
        ///
        std::vector<size_t> tiles_lut = {};
    };

    class TileMap {
    public:
        std::string name = "unnamed";
        sf::Vector2u map_size = {};
        std::vector<TileMapLayer> layers = {};

        const TileMapLayer* get_layer_by_name(const std::string& name) const;
    };
};
