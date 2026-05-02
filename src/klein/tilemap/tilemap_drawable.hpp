#pragma once
#include <memory>
#include "SFML/Graphics/Drawable.hpp"
#include "SFML/Graphics/Transformable.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/VertexBuffer.hpp"
#include "SFML/Graphics/PrimitiveType.hpp"
#include "klein/tilemap/tilemap.hpp"

namespace klein::tilemap {
    class TileMapDrawableLayer: public sf::Drawable, public sf::Transformable {
    private:
        std::shared_ptr<Spritesheet> spritesheet;
        sf::VertexBuffer buffer{sf::PrimitiveType::Triangles, sf::VertexBuffer::Usage::Static};

    public:
        TileMapDrawableLayer(std::shared_ptr<Spritesheet> tile_set);
        TileMapDrawableLayer(std::shared_ptr<Spritesheet> tile_set, const TileMapLayer &layer);

        void update(const TileMapLayer &layer);

    private:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    };

    class TileMapDrawable: public sf::Drawable, public sf::Transformable {
    public:
        std::vector<TileMapDrawableLayer> layers = {};
        TileMapDrawable(TileMapDrawableLayer layer);
        TileMapDrawable(std::vector<TileMapDrawableLayer> layers);

    private:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    };
}
