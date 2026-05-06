#include <ranges>
#include <stdexcept>
#include <unordered_map>
#include "klein/view/view_stencil.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Vertex.hpp"
#include "SFML/System/Vector2.hpp"
#include "klein/tilemap/tilemap.hpp"
#include "klein/view/raycast_impl.hpp"
#include "klein/view/view_raycast.hpp"
#include "spdlog/spdlog.h"

using std::views::zip, std::views::iota;

namespace klein::view {
    void ViewStencilState::update_staging(const RaycastViewResponse &raycast_result) {
        // TODO: do sth clever here (we could prob reuse most of the stuff in there)
        const size_t layer_count = (raycast_result.max_segments_depth + 1) * 2;
        if (layers_chunks.size() != layer_count) {
            layers_chunks.resize(layer_count);
        }
        for (auto& layer: layers_chunks) layer.clear();

        for (const auto &[ray_idx, ray]: zip(iota(0uz), raycast_result.rays)) {
            const size_t layer_cnt = ray.segments.size() + 1;
            for (int layer = 0; layer < layer_cnt; ++layer) {
                const auto view =
                    layer == 0 ?
                    raycast_result.default_view :
                    ray.segments[layer - 1].view;
                const auto distance =
                    (layer == layer_cnt - 1) ?
                    (ray.hit ? ray.hit->distance : RAYCAST_MAX_DISTANCE_TILES):
                    ray.segments[layer].distance;

                auto &layer_ref = layers_chunks[layer];
                auto [it, inserted] = layer_ref.try_emplace(view);
                auto &chunk = it->second;

                sf::Vector2f ray_origin_s = ray.origin_t.componentWiseMul(tilemap::TILE_SCREEN_SIZE);

                // just inserted -> ray origin as starting point
                // not consecutive -> push ray origin to "cap off" previous chunk part
                const bool is_consecutive = chunk._last_ray_idx == (ray_idx - 1);
                if (inserted || !is_consecutive) {
                    chunk.vertices.push_back(sf::Vertex(ray_origin_s));
                }

                sf::Vector2f segment_pos = ray.origin_t + ray.direction * distance;
                sf::Vector2f segment_pos_s = segment_pos.componentWiseMul(tilemap::TILE_SCREEN_SIZE);
                chunk.vertices.push_back(sf::Vertex(segment_pos_s));

                chunk._last_ray_idx = ray_idx;
            }
        }

        // reset vertex_count
        vertex_count = 0;
        unsigned int buffer_head = 0;
        for (auto &layer: layers_chunks) {
            for (auto& chunk: layer | std::views::values) {
                if (chunk.vertices.size() < 2) continue;

                // "close off" last chunk
                chunk.vertices.push_back(chunk.vertices[0]);

                // update vertex_count
                vertex_count += chunk.vertices.size();

                // update buffer_offset
                chunk.buffer_offset = buffer_head;
                buffer_head += chunk.vertices.size();
            }
        }
    }

    void ViewStencilState::_debug_colorize() {
        //HACK: for quick debugging
        constexpr uint8_t alpha = 255;
        constexpr std::array<sf::Color, 7> colors {
            sf::Color{255, 0, 0, alpha},
            sf::Color{0, 255, 0, alpha},
            sf::Color{0, 0, 255, alpha},
            sf::Color{255, 255, 0, alpha},
            sf::Color{255, 0, 255, alpha},
            sf::Color{0, 255, 255, alpha},
            sf::Color{255, 255, 255, alpha}
        };
        // assign each seen viewkey a sequential index
        std::unordered_map<ViewKey, size_t, ViewKeyHash> viewkey_indices{};
        size_t index = 0;
        for (const auto &layer: layers_chunks) {
            for (const auto &[viewkey, chunk]: layer) {
                if (viewkey_indices.contains(viewkey)) continue;
                viewkey_indices[viewkey] = index++;
            }
        }
        //update debug color based on it
        for (auto &layer: layers_chunks) {
            for (auto &[viewkey, chunk]: layer) {
                const auto color_index = viewkey_indices[viewkey] % colors.size();
                for (auto &vertex: chunk.vertices) vertex.color = colors[color_index];
            }
        }
    }

    void ViewStencilState::upload_staging() {
        // grow buffer to fit
        size_t desired_size = vertex_count ? std::bit_ceil(vertex_count) : 0;
        if (buffer.getVertexCount() < desired_size) {
            spdlog::info("growing view stencil buffer to {}", desired_size);
            if (!buffer.create(desired_size))
                throw std::runtime_error("VertexBuffer::create failed");
        }

        // upload
        for (const auto& layer: layers_chunks) {
            for (const auto& chunk: layer | std::views::values) {
                if (chunk.vertices.size() < 3) continue;
                if (!buffer.update(chunk.vertices.data(), chunk.vertices.size(), chunk.buffer_offset))
                    throw std::runtime_error("VertexBuffer::update failed");
            }
        }
    }

    void ViewStencilState::_debug_draw(sf::RenderTarget &target) const {
        for (const auto &layer: layers_chunks | std::views::reverse) {
            for (const auto& chunk: layer | std::views::values) {
                if (chunk.vertices.size() < 3) continue;
                target.draw(buffer, chunk.buffer_offset, chunk.vertices.size());
            }
        }
    }

    // draw the stuff to stencil only
    void ViewStencilState::draw(sf::RenderTarget &target) const {
        // TODO
    }

}
