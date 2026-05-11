#pragma once

#include <string>
#include <vector>
#include <span>
#include <unordered_map>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Vector2.hpp>

namespace klein::animation {
    struct AnimationMeta {
        std::string name;
        std::vector<sf::IntRect> frames{};
        unsigned int framerate = 0;
        int priority = 0;
    };

    enum class AnimationType {
        Loop,
        Sustain,
        Oneshot
    };

    struct AnimationState {
        std::string name;
        AnimationType type = AnimationType::Loop;
        sf::Clock clock {};
    };

    class AnimationDrawable: public sf::Drawable {
    private:
        std::unordered_map<std::string, AnimationMeta> animations {};
        std::vector<AnimationState> animation_stack {};

    public:
        sf::Texture texture;
        sf::Vector2f origin;
        sf::Vector2f scale { 1.f, 1.f };

        inline AnimationDrawable() = delete;
        AnimationDrawable(sf::Texture texture, std::span<const AnimationMeta>);

        void toggle_animation(const std::string &animation, bool state, AnimationType type = AnimationType::Loop);
        void push_animation(const std::string &animation, AnimationType type = AnimationType::Loop);
        bool pop_animation(const std::string &animation);

    private:
        sf::IntRect resolve_rect() const;
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    };

}
