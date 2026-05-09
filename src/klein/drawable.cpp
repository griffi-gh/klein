#include "klein/drawable.hpp"

#include <memory>
#include <entt/entity/fwd.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderStates.hpp>

namespace klein::drawable {
    void render_drawable(
        entt::registry &registry,
        sf::RenderTarget &sf_target,
        const entt::entity entity,
        const sf::RenderStates& states
    ) {
        const auto &drawable = registry.get<const drawable_ptr>(entity);
        const sf::Drawable& sf_drawable = *drawable.get();

        sf::RenderStates states_copy(states);
        if (auto *transform_ptr = registry.try_get<sf::Transform>(entity))
            states_copy.transform = *transform_ptr * states.transform;

        sf_target.draw(sf_drawable, states_copy);
    }

    void render_drawable(
        entt::registry &registry,
        sf::RenderTarget &target,
        entt::const_runtime_view view,
        const sf::RenderStates& states
    ) {
        view.iterate(registry.storage<drawable_ptr>());
        for (const auto entity: view)
            render_drawable(registry, target, entity, states);
    }
}
