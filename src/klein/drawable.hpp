#pragma once
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <entt/entt.hpp>

namespace klein::drawable {
    using drawable_ptr = std::unique_ptr<sf::Drawable>;

    void render_drawable(
        const entt::registry &registry,
        sf::RenderTarget &target,
        entt::entity entity,
        const sf::RenderStates& states = {}
    );
    void render_drawable(
        entt::registry &registry, // (cannot be const due to .storage() usage)
        sf::RenderTarget &target,
        entt::const_runtime_view view,
        const sf::RenderStates& states = {}
    );
}
