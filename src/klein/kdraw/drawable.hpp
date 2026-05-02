#pragma once
#include "SFML/Graphics/Drawable.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/Transform.hpp"
#include "entt/entt.hpp"
#include <memory>

namespace klein::kdraw {
    struct Drawable {
        std::unique_ptr<sf::Drawable> sf_drawable;
    };

    struct Transform {
        sf::Transform sf_transform = {};
    };

    void render_drawables(entt::registry &registry, sf::RenderTarget &target);
}
