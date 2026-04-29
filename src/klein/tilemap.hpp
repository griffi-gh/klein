#pragma once
#include <span>
#include <memory>
#include "SFML/Graphics/Drawable.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/Transformable.hpp"
#include "SFML/Graphics/VertexArray.hpp"
#include "SFML/System/Vector2.hpp"

namespace klein {
    class TileSetAsset {
    public:
        sf::Texture texture;
        sf::Vector2u tile_size;
    };

    struct TileMapLayerData {
        std::span<const int> map;
        unsigned int width;
        unsigned int height;
    };

    class TileMapLayerDrawable: public sf::Drawable, public sf::Transformable {
    private:
        std::shared_ptr<TileSetAsset> tile_set;
        sf::VertexArray vertices = {};

    public:
        TileMapLayerDrawable(std::shared_ptr<TileSetAsset> asset);
        TileMapLayerDrawable(std::shared_ptr<TileSetAsset> asset, const TileMapLayerData &data);
        void set_layer_data(const TileMapLayerData &data);

    private:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    };

};
