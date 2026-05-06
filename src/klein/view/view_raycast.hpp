#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <entt/entt.hpp>

#include "klein/view/raycast_impl.hpp"

namespace klein::view {
    // XXX: i am aware hashing floats is a bad idea
    // quite frankly i dont give a fk though,
    // they come from same literals either way, so *in practice* should always hash to same value
    // juuuust following the "if it works, it works, don't touch it" principle here :p
    struct ViewKey {
        sf::Vector2f trans {0, 0};
        sf::Vector2f scale {1, 1};
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

    struct RaycastViewResponse {
        ViewKey default_view;
        std::vector<RayPath> rays = {};
        // cache:
        std::unordered_set<ViewKey, ViewKeyHash> unique_views = { };
        std::unordered_map<ViewKey, uint8_t, ViewKeyHash> view_stencil_map = { };
        size_t max_segments_depth = 0;

        void draw_debug(sf::RenderTarget &target) const;
    };

    constexpr int VIEW_RAY_COUNT = 500;

    RaycastViewResponse raycast_view(entt::registry &registry);
}
