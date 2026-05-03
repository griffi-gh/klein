#pragma once
#include "SFML/Graphics/RenderTarget.hpp"
#include "entt/entt.hpp"

namespace klein {
    void raytrace_portals(entt::registry &registry, sf::RenderTarget &target);
}
