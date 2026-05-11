#include "klein/animation/animation.hpp"

#include <SFML/System/Time.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

// this is overengineered

namespace klein::animation {
    static bool is_expired(const AnimationState &state, const AnimationMeta &meta, sf::Time elapsed) {
        return
            state.type == AnimationType::Oneshot &&
            meta.framerate != 0 &&
            elapsed.asSeconds() > (float)meta.frames.size() / (float)meta.framerate;
    }

    AnimationDrawable::AnimationDrawable(
        const sf::Texture texture,
        const std::span<const AnimationMeta> animations
    ): texture(std::move(texture)) {
        for (const auto& animation: animations)
            this->animations[animation.name] = animation;
    }

    void AnimationDrawable::toggle_animation(const std::string& animation, const bool state, AnimationType type) {
        if (state) {
            push_animation(animation, type);
        } else {
            pop_animation(animation);
        }
    }

    void AnimationDrawable::push_animation(const std::string& animation, AnimationType type) {
        const AnimationState state(animation, type);

        // remove expired + find existing animation
        for (auto it = animation_stack.begin(); it != animation_stack.end(); ++it) {
            const auto &meta = animations.at(it->name);
            if (is_expired(*it, meta, it->clock.getElapsedTime())) {
                animation_stack.erase(it--);
                continue;
            }
            if (it->name == animation) return;
        }

        const auto &meta = animations.at(animation);
        for (auto it = animation_stack.begin(); it != animation_stack.end(); ++it) {
            const auto &it_meta = animations.at(it->name);
            if (it_meta.priority <= meta.priority) continue;

            animation_stack.insert(it, state);
            return;
        }
        animation_stack.emplace_back(state);
    }

    bool AnimationDrawable::pop_animation(const std::string &animation) {
        for (auto it = animation_stack.begin(); it != animation_stack.end(); ++it) {
            if (it->name != animation) continue;
            animation_stack.erase(it);
            return true;
        }
        return false;
    }

    sf::IntRect AnimationDrawable::resolve_rect() const {
        if (animation_stack.empty()) return {};

        const AnimationState *state;
        const AnimationMeta *meta;
        sf::Time elapsed;
        for (const auto &maybe_state: animation_stack | std::views::reverse) {
            state = &maybe_state;
            meta = &animations.at(state->name);
            elapsed = maybe_state.clock.getElapsedTime();

            // check if oneshot and expired
            if (is_expired(*state, *meta, elapsed)) continue;

            break;
        }

        if (meta->frames.empty()) return {};
        if (meta->framerate == 0) return meta->frames[0];

        const float frame_delay = 1. / (float)meta->framerate;
        const size_t frame_unbounded = static_cast<size_t>(std::floor(elapsed.asSeconds() / frame_delay));
        const size_t frame = state->type == AnimationType::Loop
            ? (frame_unbounded % meta->frames.size())
            : std::min(frame_unbounded, meta->frames.size() - 1);

        return meta->frames[frame];
    }

    void AnimationDrawable::draw(sf::RenderTarget& target, const sf::RenderStates states) const {
        const sf::IntRect rect = resolve_rect();
        sf::Sprite sprite(texture, rect);
        sprite.setOrigin(origin);
        sprite.setScale(scale);
        target.draw(sprite, states);
    }
}
