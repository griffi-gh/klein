#include "klein/tilemap/tilemap_loader.hpp"

#include <cstdint>
#include <fstream>
#include <SFML/System/Vector2.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <zlib.h>

#include "klein/tilemap/tilemap.hpp"
#include "klein/vfs/assets.hpp"

using json = nlohmann::json;

namespace klein::tilemap {
    TileMap load_tile_map_data(std::string asset, std::string name, bool compressed) {
        spdlog::info("loading map data for \"{}\" ({}, {})",
            name, asset, compressed ? "compressed" : "raw");

        auto asset_path = vfs::asset_path(asset);

        std::string json_data;

        if (compressed) {
            gzFile file = gzopen(asset_path.c_str(), "rb");
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
                    .attributes = tile_data["attributes"].is_null()
                        ? std::nullopt
                        : std::make_optional(tile_data["attributes"]),
                };
                layer.tiles.push_back(tile);
            }
            layer.update_tiles();
            map.layers.push_back(layer);
        }

        return map;
    }
}
