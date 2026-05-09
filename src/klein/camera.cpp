#include "klein/camera.hpp"

#include <entt/entt.hpp>
#include <SFML/Graphics/Transform.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Window/ContextSettings.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace klein::camera {
    void Camera2d::update_view() {
        sf::Vector2f vp_size;

        const float aspect = (float)viewport.getSize().x / (float)viewport.getSize().y;
        switch (config.base_size_option) {
            case SizeOption::None:
                vp_size = sf::Vector2f(viewport.getSize()) * config.view_scale;
                break;
            case SizeOption::FitInside:
                if (aspect > 1.) {
                    vp_size = sf::Vector2f(config.base_size.x, config.base_size.x / aspect) * config.view_scale;
                } else {
                    vp_size = sf::Vector2f(config.base_size.y * aspect, config.base_size.y) * config.view_scale;
                }
                break;
            case SizeOption::ConstantWidth:
                vp_size = sf::Vector2f(config.base_size.x, config.base_size.x / aspect) * config.view_scale;
                break;
            case SizeOption::ConstantHeight:
                vp_size = sf::Vector2f(config.base_size.y * aspect, config.base_size.y) * config.view_scale;
                break;
            case SizeOption::Stretch:
                vp_size = sf::Vector2f(viewport.getSize()) * config.view_scale;
                break;
        }

        sf::View view(center_pos, vp_size);
        viewport.setView(view);
    }

    void Camera2d::update(const entt::registry& registry, sf::Time dt) {
        const sf::Vector2f subject_pos = registry.get<sf::Transform>(subject).transformPoint({});

        sf::Vector2f target_pos = config.leeway_enable
            ? sf::Vector2f(
                std::clamp(center_pos.x, subject_pos.x - config.leeway.x, subject_pos.x + config.leeway.x),
                std::clamp(center_pos.y, subject_pos.y - config.leeway.y, subject_pos.y + config.leeway.y))
            : subject_pos;

        if (config.smooth_enable) {
            center_pos += dt.asSeconds() * config.smooth_fac * (target_pos - center_pos);
        } else {
            center_pos = target_pos;
        }
        update_view();
    }

    void Camera2d::resize(const sf::Vector2u resolution, const sf::ContextSettings settings) {
        if (viewport.getSize() == resolution) return;
        if (!viewport.resize(resolution, settings))
            throw std::runtime_error("Failed to resize viewport");
        update_view();
    }

    void Camera2d::display() {
        viewport.display();
    }
}
