#pragma once
#include "SFML/Graphics/Drawable.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "entt/entt.hpp"

namespace klein::kdraw {
    void render_drawables(entt::registry &registry, sf::RenderTarget &target);
}
