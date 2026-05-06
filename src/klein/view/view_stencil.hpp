#pragma once
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
    };

    class ViewStencilState {
    private:
        sf::VertexBuffer buffer{ sf::PrimitiveType::TriangleFan, sf::VertexBuffer::Usage::Stream };

        // maps
        // [layer][viewkey] -> std::vector<sf::Vertex>
        std::vector<std::unordered_map<ViewKey, ViewStencilChunk, ViewKeyHash>> layers_chunks;

    public:
        /// update the stencil vertex buffers based on the raycasts
        ///
        void update(const RaycastViewResponse &raycast_result);

        /// draw the buffer to the specified target
        ///
        /// the target (window/texture) needs to have a stencil buffer
        ///
        void draw(sf::RenderTarget &target) const;
    };

}
