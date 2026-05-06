#include "klein/tilemap/tilemap_drawable.hpp"
#include "SFML/Graphics/Vertex.hpp"
#include "SFML/Graphics/VertexBuffer.hpp"
#include "klein/tilemap/tilemap.hpp"

using std::views::reverse;

namespace klein::tilemap {
    TileMapDrawableLayer::TileMapDrawableLayer(std::shared_ptr<Spritesheet> tile_set)
        : spritesheet(std::move(tile_set)) {}

    TileMapDrawableLayer::TileMapDrawableLayer(
        std::shared_ptr<Spritesheet> tile_set,
        const TileMapLayer &layer
    )
        : spritesheet(std::move(tile_set))
    {
        update(layer);
    }

    // built using this example as a general reference, rewrote it using VertexBuffer though
    // https://www.sfml-dev.org/tutorials/3.0/graphics/vertex-array/#example-tile-map
    void TileMapDrawableLayer::update(const TileMapLayer &layer) {
        if (layer.tiles.size() == 0) {
            if (!buffer.create(0))
                throw std::runtime_error("VertexBuffer::create failed");
            return;
        }

        std::vector<sf::Vertex> vertices;
        vertices.reserve(layer.tiles.size() * 6);

        const int tiles_per_row = spritesheet->texture.getSize().x / spritesheet->tile_size.x;

        for (const auto &tile: layer.tiles) {
            const int x = tile.pos.x;
            const int y = tile.pos.y;
            const int tex_x = tile.tex_id % tiles_per_row;
            const int tex_y = tile.tex_id / tiles_per_row;

            vertices.insert(vertices.end(), std::initializer_list<sf::Vertex>{
                {
                    .position = sf::Vector2f(x * spritesheet->tile_size.x, y * spritesheet->tile_size.y),
                    .texCoords = sf::Vector2f(tex_x * spritesheet->tile_size.x, tex_y * spritesheet->tile_size.y),
                },
                {
                    .position = sf::Vector2f((x + 1) * spritesheet->tile_size.x, y * spritesheet->tile_size.y),
                    .texCoords = sf::Vector2f((tex_x + 1) * spritesheet->tile_size.x, tex_y * spritesheet->tile_size.y),
                },
                {
                    .position = sf::Vector2f(x * spritesheet->tile_size.x, (y + 1) * spritesheet->tile_size.y),
                    .texCoords = sf::Vector2f(tex_x * spritesheet->tile_size.x, (tex_y + 1) * spritesheet->tile_size.y),
                },
                {
                    .position = sf::Vector2f(x * spritesheet->tile_size.x, (y + 1) * spritesheet->tile_size.y),
                    .texCoords = sf::Vector2f(tex_x * spritesheet->tile_size.x, (tex_y + 1) * spritesheet->tile_size.y),
                },
                {
                    .position = sf::Vector2f((x + 1) * spritesheet->tile_size.x, y * spritesheet->tile_size.y),
                    .texCoords = sf::Vector2f((tex_x + 1) * spritesheet->tile_size.x, tex_y * spritesheet->tile_size.y),
                },
                {
                    .position = sf::Vector2f((x + 1) * spritesheet->tile_size.x, (y + 1) * spritesheet->tile_size.y),
                    .texCoords = sf::Vector2f((tex_x + 1) * spritesheet->tile_size.x, (tex_y + 1) * spritesheet->tile_size.y),
                }
            });
        }


        // TODO reuse existing buffer if possible
        assert(sf::VertexBuffer::isAvailable());
        assert(vertices.size() != 0);
        if (!buffer.create(vertices.size()))
            throw std::runtime_error("VertexBuffer::create failed");
        if (!buffer.update(vertices.data(), vertices.size(), 0))
            throw std::runtime_error("VertexBuffer::update failed");
    }

    void TileMapDrawableLayer::draw(sf::RenderTarget& target, sf::RenderStates states) const {
        states.transform *= getTransform();
        states.texture = &spritesheet->texture;
        target.draw(buffer, states);
    }

    TileMapDrawable::TileMapDrawable(std::vector<TileMapDrawableLayer> layers)
        :layers(std::move(layers)) {}

    TileMapDrawable::TileMapDrawable(std::shared_ptr<Spritesheet> tile_set, const TileMap &map) {
        layers.reserve(map.layers.size());
        for (const auto &layer: map.layers) {
            layers.push_back(TileMapDrawableLayer(tile_set, layer));
        }
    }

    void TileMapDrawable::draw(sf::RenderTarget& target, sf::RenderStates states) const {
        for (const auto &layer: layers | reverse) {
            target.draw(layer, states);
        }
    }
}
