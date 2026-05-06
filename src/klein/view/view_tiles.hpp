#pragma once

#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>

#include "klein/view/view_stencil.hpp"

namespace klein::view {
    void render_views(entt::registry& registry, const ViewStencilState& stencil_state);
}
