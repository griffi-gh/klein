#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <entt/entt.hpp>

#include "klein/view/raycast_impl.hpp"

namespace klein::view {
    constexpr int VIEW_RAY_COUNT = 1024;

    // XXX: i am aware hashing floats is a bad idea
    // quite frankly i dont give a fk though,
    // they come from same literals either way, so *in practice* should always hash to same value
    // juuuust following the "if it works, it works, don't touch it" principle here :p
    struct ViewKey {
        sf::Vector2f trans {0, 0};
        // sf::Vector2f scale {1, 1}; // (currently unused)
        ViewKey operator*(const ViewKey&) const noexcept;
        bool operator==(const ViewKey&) const noexcept = default;
    };
    struct ViewKeyHash {
        size_t operator()(const ViewKey& k) const noexcept;
    };

    struct RayTransition {
        ViewKey view;
        float distance;
        sf::Vector2i tile;
    };

    struct RayPath {
        sf::Vector2f origin_t;
        sf::Vector2f direction;
        std::vector<RayTransition> segments;
        std::optional<Hit> hit;
    };

    struct ViewMeta {
        uint8_t stencil_idx = 0;
        sf::Vector2i visible_aabb_min { INT_MAX, INT_MAX };
        sf::Vector2i visible_aabb_max { INT_MIN, INT_MIN };
    };

    struct RaycastViewResponse {
        ViewKey default_view;
        std::vector<RayPath> rays {};
        std::unordered_map<ViewKey, ViewMeta, ViewKeyHash> views {};

        size_t max_segments_depth = 0;

        void draw_debug(sf::RenderTarget &target) const;
    };

    RaycastViewResponse raycast_view(entt::registry &registry);
}
