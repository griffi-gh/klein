#pragma once

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <entt/entt.hpp>

namespace klein::drawable {
    // HACK: i know i should've used entt::poly for this

    using draw_fn = void(*)(
        const entt::registry&,
        entt::entity,
        sf::RenderTarget&,
        sf::RenderStates
    );

    template <typename... Ts>
    inline void draw_components(
        const entt::registry& registry,
        entt::entity entity,
        sf::RenderTarget& target,
        sf::RenderStates states
    ) {
        (..., target.draw(
            static_cast<const sf::Drawable&>(
                registry.get<const Ts>(entity)
            ),
            states
        ));
    }

    struct Drawable {
        draw_fn draw;
        inline Drawable() = delete;
        inline Drawable(const draw_fn draw): draw(draw) {}
    };

    void draw_entity(
        const entt::registry &registry,
        entt::entity entity,
        sf::RenderTarget &target,
        const sf::RenderStates& states = {}
    );
}
