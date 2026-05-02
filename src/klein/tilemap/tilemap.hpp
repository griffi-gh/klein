#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "SFML/Graphics/Rect.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/System/Vector2.hpp"

namespace klein::tilemap {
    class Spritesheet {
    public:
        sf::Texture texture;
        sf::Vector2u tile_size;
    };

    struct Tile {
        sf::Vector2i pos; /**< Global position (NOT offset by layer's aabb_origin */
        uint16_t tex_id;
        // TODO: attributes (json map in fmt)
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
        void update_tiles();

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
    };
};
