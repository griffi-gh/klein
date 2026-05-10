#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>

namespace klein::tilemap {
    constexpr sf::Vector2f TILE_SCREEN_SIZE { 32, 32 };

    constexpr std::string LAYER_SPECIAL = "_special";

    class Spritesheet {
    public:
        sf::Texture texture;
        sf::Vector2u tile_size;
    };

    struct TileBase {
        static constexpr std::string_view type = "";
    };
    struct TilePortal: TileBase {
        static constexpr std::string_view type = "portal";
        float trans_x = 0.f;
        float trans_y = 0.f;
        float scale_x = 0.f;
        float scale_y = 0.f;
        // flags
        uint8_t face_mask: 4 = 0;
        bool flip_v: 1 = 0;
        bool flip_h: 1 = 0;
    };
    struct TileHard: TileBase {
        static constexpr std::string_view type = "hard";
    };
    struct TileSoft: TileBase {
        static constexpr std::string_view type = "soft";
    };
    struct TilePlayerSpawn: TileBase {
        static constexpr std::string_view type = "player_spawn";
    };

    using TileAttributes = std::variant<
        TileBase,
        TilePortal,
        TileHard,
        TileSoft,
        TilePlayerSpawn>;

    struct Tile {
        sf::Vector2i pos; /**< Global position (NOT offset by layer's aabb_origin */
        uint16_t tex_id;
        TileAttributes attributes = TileBase{};
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
        /// Updates internal layer lookup table and aabb
        /// (required for TileMapLayer::get to work)
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
