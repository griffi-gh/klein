#include "klein/tilemap/tilemap_loader.hpp"

#include <string>
#include <cstdint>
#include <fstream>
#include <SFML/System/Vector2.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <zlib.h>

#include "klein/tilemap/tilemap.hpp"
#include "klein/vfs/vfs_assets.hpp"

using json = nlohmann::json;

namespace klein::tilemap {
    TileAttributes parse_tile_attributes(const nlohmann::json &attributes) {
        if (attributes.is_null() || !attributes["type"].is_string())
            return TileBase {};

        const std::string &decl_type = attributes["type"];
        if (decl_type == TileHard::type)
            return TileHard{};
        if (decl_type == TileSoft::type)
            return TileSoft{};
        if (decl_type == TilePortal::type)
            return TilePortal {
                .trans_x = attributes["trans_x"],
                .trans_y = attributes["trans_y"],
                .face_mask = attributes["face_mask"],
                .flip_v = attributes["flip_v"].get<uint8_t>() != 0,
                .flip_h = attributes["flip_h"].get<uint8_t>() != 0,
            };
        if (decl_type == TilePlayerSpawn::type)
            return TilePlayerSpawn{};

        return TileBase {};
    }

    TileMap load_tile_map_data(
        const std::filesystem::path &asset,
        const std::string_view name,
        const bool compressed
    ) {
        spdlog::info("loading map data for \"{}\" ({}, {})",
            name, asset.string(), compressed ? "compressed" : "raw");

        const auto asset_path = vfs::resolve_asset_path(asset);

        std::string json_data;

        if (compressed) {
#if defined(_WIN32)
            const gzFile file = gzopen_w(asset_path.c_str(), "rb");
#else
            const gzFile file = gzopen(asset_path.c_str(), "rb");
#endif
            if (!file) throw std::runtime_error("gzopen failed");

            char buffer[4096];
            while (true) {
                int bytes = gzread(file, buffer, sizeof(buffer));
                if (bytes > 0) {
                    json_data.append(buffer, bytes);
                } else if (bytes == 0) {
                    break;
                } else {
                    int err;
                    const char* msg = gzerror(file, &err);
                    gzclose(file);
                    throw std::runtime_error(msg ? msg : "gzread failed");
                }
            }
            gzclose(file);
        } else {
            std::ifstream file(asset);
            if (!file.is_open()) throw std::runtime_error("ifstream failed");
            json_data.assign(
                (std::istreambuf_iterator<char>(file)),
                std::istreambuf_iterator<char>()
            );
            if (file.bad()) throw std::runtime_error("ifstream read failed");
            file.close();
        }

        auto data = nlohmann::json::parse(json_data);

        TileMap map{};
        map.name = name;
        map.map_size = sf::Vector2u(
            data["mapWidth"].get<unsigned int>(),
            data["mapHeight"].get<unsigned int>()
        );
        map.layers.reserve(data["layers"].size());
        for (auto& layer_data: data["layers"]) {
            TileMapLayer layer{};
            layer.name = layer_data["name"];
            layer.collider = layer_data["collider"];
            layer.tiles.reserve(layer_data["tiles"].size());
            for (auto& tile_data: layer_data["tiles"]) {
                // idk why the fvck spritefusion serializes id as a string
                auto sprite_id = std::stoul(tile_data["id"].get<std::string>());
                Tile tile{
                    .pos = sf::Vector2i(
                        tile_data["x"].get<unsigned int>(),
                        tile_data["y"].get<unsigned int>()
                    ),
                    .tex_id = static_cast<uint16_t>(sprite_id),
                    .attributes = parse_tile_attributes(tile_data["attributes"]),
                };
                layer.tiles.push_back(tile);
            }
            layer.update_tiles();
            map.layers.push_back(layer);
        }

        return map;
    }
}
