#include "tilemap.hpp"

klein::TileMapLayerDrawable::TileMapLayerDrawable(
    std::shared_ptr<TileSetAsset> tile_set
)
: tile_set(std::move(tile_set)) {}

klein::TileMapLayerDrawable::TileMapLayerDrawable(
    std::shared_ptr<TileSetAsset> tile_set,
    const TileMapLayerData &data
)
: tile_set(std::move(tile_set)) {
    set_layer_data(data);
}

// heavily based on the example from:
// https://www.sfml-dev.org/tutorials/3.0/graphics/vertex-array/#example-tile-map

void klein::TileMapLayerDrawable::set_layer_data(const TileMapLayerData &data) {
    assert(tile_set);
    assert(tile_set->tile_size.x > 0 && tile_set->tile_size.y > 0);
    assert(data.map.size() == data.width * data.height);

    vertices.setPrimitiveType(sf::PrimitiveType::Triangles); // N.B. Quads got removed in sfml3
    vertices.resize(data.width * data.height * 6);

    const int tiles_per_row = tile_set->texture.getSize().x / tile_set->tile_size.x;

    for (unsigned int y = 0; y < data.height; ++y) {
        for (unsigned int x = 0; x < data.width; ++x) {
            const size_t map_idx = x + y * data.width;
            const size_t tile_idx = data.map[map_idx];
            const int tile_x = tile_idx % tiles_per_row;
            const int tile_y = tile_idx / tiles_per_row;
            const size_t vertex_idx = map_idx * 6;
            vertices[vertex_idx] = {
                .position = sf::Vector2f(x * tile_set->tile_size.x, y * tile_set->tile_size.y),
                .texCoords = sf::Vector2f(tile_x * tile_set->tile_size.x, tile_y * tile_set->tile_size.y),
            };
            vertices[vertex_idx + 1] = {
                .position = sf::Vector2f((x + 1) * tile_set->tile_size.x, y * tile_set->tile_size.y),
                .texCoords = sf::Vector2f((tile_x + 1) * tile_set->tile_size.x, tile_y * tile_set->tile_size.y),
            };
            vertices[vertex_idx + 2] = {
                .position = sf::Vector2f(x * tile_set->tile_size.x, (y + 1) * tile_set->tile_size.y),
                .texCoords = sf::Vector2f(tile_x * tile_set->tile_size.x, (tile_y + 1) * tile_set->tile_size.y),
            };
            vertices[vertex_idx + 3] = {
                .position = sf::Vector2f(x * tile_set->tile_size.x, (y + 1) * tile_set->tile_size.y),
                .texCoords = sf::Vector2f(tile_x * tile_set->tile_size.x, (tile_y + 1) * tile_set->tile_size.y),
            };
            vertices[vertex_idx + 4] = {
                .position = sf::Vector2f((x + 1) * tile_set->tile_size.x, y * tile_set->tile_size.y),
                .texCoords = sf::Vector2f((tile_x + 1) * tile_set->tile_size.x, tile_y * tile_set->tile_size.y),
            };
            vertices[vertex_idx + 5] = {
                .position = sf::Vector2f((x + 1) * tile_set->tile_size.x, (y + 1) * tile_set->tile_size.y),
                .texCoords = sf::Vector2f((tile_x + 1) * tile_set->tile_size.x, (tile_y + 1) * tile_set->tile_size.y),
            };
        }
    }
}

void klein::TileMapLayerDrawable::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= getTransform();
    states.texture = &tile_set->texture;
    target.draw(vertices, states);
}
