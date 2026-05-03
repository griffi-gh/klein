#include "drawable.hpp"

namespace klein {
    void render_drawable(entt::registry &registry, sf::RenderTarget &sf_target) {
        auto view = registry.view<std::unique_ptr<sf::Drawable>>();
        view.each([&registry, &sf_target](entt::entity entity, const auto &drawable) {
            sf::Transform sf_transform{};
            if (auto *transform_ptr = registry.try_get<sf::Transform>(entity))
                sf_transform = *transform_ptr;
            const sf::Drawable& sf_drawable = *drawable.get();
            sf_target.draw(sf_drawable, sf_transform);
        });
    }
}
