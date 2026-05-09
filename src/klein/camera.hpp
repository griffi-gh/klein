#pragma once
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Window/ContextSettings.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <entt/fwd.hpp>

namespace klein::camera {
    enum class SizeOption {
        None,
        FitInside,
        ConstantWidth,
        ConstantHeight,
        Stretch,
    };

    struct CameraConfig {
        bool smooth_enable = false;
        float smooth_fac = 1.0;

        bool leeway_enable = false;
        sf::Vector2f leeway { };

        SizeOption base_size_option = SizeOption::None;
        sf::Vector2f base_size {};

        float view_scale = 1.0;
    };

    class Camera2d {
    private:
        sf::RenderTexture viewport {};
        sf::Vector2f center_pos {};
        void update_view();

    public:
        entt::entity subject {};
        CameraConfig config {};

        inline Camera2d() = default;
        inline Camera2d(CameraConfig config): config(config) {}

        void update(const entt::registry& registry, sf::Time dt);

        void resize(sf::Vector2u resolution, sf::ContextSettings settings = {});
        void display();

        inline sf::RenderTarget& render_target() { return viewport; }
        inline const sf::RenderTarget& render_target() const { return viewport; }
        inline const sf::Texture& texture() const { return viewport.getTexture(); }
    };
}
