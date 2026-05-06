#pragma once
#include <cstdint>
#include <unordered_map>
#include <vector>
#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/Vertex.hpp"
#include "SFML/Graphics/VertexBuffer.hpp"
#include "klein/view/view_raycast.hpp"

namespace klein::view {
    struct ViewStencilChunk {
        std::vector<sf::Vertex> vertices{};
        unsigned int buffer_offset;

        // implementation detail
        size_t _last_ray_idx = SIZE_MAX;
    };

    class ViewStencilState {
    private:
        sf::VertexBuffer buffer{ sf::PrimitiveType::TriangleFan, sf::VertexBuffer::Usage::Stream };

        // maps
        // [layer][viewkey] -> std::vector<sf::Vertex>
        std::vector<std::unordered_map<ViewKey, ViewStencilChunk, ViewKeyHash>> layers_chunks;

        size_t vertex_count = 0;

    public:
        /// update the stencil vertex buffers based on the raycasts
        ///
        void update_staging(const RaycastViewResponse &raycast_result);

        /// call between update_staging and upload_staging to get colorized segments based on viewkey
        void _debug_colorize();

        /// upload staging buffer(s) created by update_staging to the GPU
        ///
        void upload_staging();

        /// draw the segments as vertices
        ///
        /// requires _debug_colorize to be actually useful
        void _debug_draw(sf::RenderTarget &target) const;

        /// draw the buffer to the specified target's stencil buffer
        ///
        /// the target (window/texture) needs to have a stencil buffer
        ///
        void draw(sf::RenderTarget &target) const;


    };

}
