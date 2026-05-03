#pragma once
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/System/Vector2.hpp"
#include "entt/entt.hpp"

namespace klein {
    // XXX: i am aware hashing floats is a bad idea
    // quite frankly i dont give a fvck though,
    // they come from same literals either way so in practice should always hash to same value
    struct ViewKey {
        sf::Vector2f trans{};
        sf::Vector2f scale{};
        bool operator==(const ViewKey&) const = default;
    };
    struct ViewKeyHash {
        size_t operator()(const klein::ViewKey& k) const noexcept;
    };

    void raytrace_portals(entt::registry &registry, sf::RenderTarget &target);
}
