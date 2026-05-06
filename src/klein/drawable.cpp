#include "klein/drawable.hpp"

#include <memory>
#include <entt/entity/fwd.hpp>
#include <SFML/Graphics/Drawable.hpp>

namespace klein {
    void render_drawable(
        entt::registry &registry,
        sf::RenderTarget &sf_target,
        entt::const_runtime_view view
    ) {
        view.iterate(registry.storage<drawable_ptr>());
        for (auto entity: view) {
            const auto &drawable = registry.get<const drawable_ptr>(entity);
            const sf::Drawable& sf_drawable = *drawable.get();

            sf::Transform sf_transform{};
            if (auto *transform_ptr = registry.try_get<sf::Transform>(entity))
                sf_transform = *transform_ptr;

            sf_target.draw(sf_drawable, sf_transform);
        }
    }
}
