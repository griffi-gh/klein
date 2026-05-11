#include "klein/drawable.hpp"

#include <entt/entity/fwd.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderStates.hpp>

namespace klein::drawable {
    void draw_entity(
        const entt::registry &registry,
        const entt::entity entity,
        sf::RenderTarget &target,
        const sf::RenderStates& states
    ) {
        const auto *drawable_ptr = registry.try_get<const Drawable>(entity);
        if (!drawable_ptr) return;

        sf::RenderStates states_copy = states;
        if (const auto *transform_ptr = registry.try_get<const sf::Transform>(entity))
            states_copy.transform = *transform_ptr * states.transform;

        drawable_ptr->draw(registry, entity, target, states_copy);
    }
}
