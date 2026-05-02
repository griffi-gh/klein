#include "drawable.hpp"

namespace klein::kdraw {
    void render_drawables(entt::registry &registry, sf::RenderTarget &sf_target) {
        auto view = registry.view<Drawable>();
        view.each([&registry, &sf_target](entt::entity entity, const auto &drawable) {
            sf::Transform sf_transform{};
            if (auto *transform_ptr = registry.try_get<Transform>(entity)) {
                sf_transform = transform_ptr->sf_transform;
            }

            const sf::Drawable& sf_drawable = *drawable.sf_drawable.get();
            sf_target.draw(sf_drawable, sf_transform);
        });
    }
}
