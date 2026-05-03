#pragma once
#include "SFML/Graphics/Drawable.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "entt/entt.hpp"

namespace klein {
    void render_drawable(entt::registry &registry, sf::RenderTarget &target);
}
