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

using std::views::zip,
    std::views::iota,
    std::views::reverse;

namespace klein::view {
    void ViewStencilState::update_staging(const RaycastViewResponse &raycast_result) {
        // TODO: do sth clever here (we could prob reuse most of the stuff in there)
        const size_t layer_count = (raycast_result.max_segments_depth + 1) * 2;
        if (layers_chunks.size() != layer_count) {
            layers_chunks.resize(layer_count);
        }
        for (auto& layer: layers_chunks) layer.clear();

        vertex_count = 0;

        const auto handle_segment = [&](
            const RayPath &ray,
            size_t layer,
            ViewKey viewkey,
            float distance
        ) {
            auto &layer_ref = layers_chunks[layer];
            auto [it, inserted] = layer_ref.try_emplace(viewkey);
            auto &chunk = it->second;
            if (inserted) {
                // if just inserted insert the fan center
                sf::Vector2f ray_origin_s = ray.origin_t.componentWiseMul(tilemap::TILE_SCREEN_SIZE);
                chunk.vertices.push_back(sf::Vertex(ray_origin_s));
                vertex_count += 1;
            }

            sf::Vector2f segment_pos = ray.origin_t + ray.direction * distance;
            sf::Vector2f segment_pos_s = segment_pos.componentWiseMul(tilemap::TILE_SCREEN_SIZE);
            chunk.vertices.push_back(sf::Vertex(segment_pos_s));
            vertex_count += 1;
        };

        for (auto &ray: raycast_result.rays) {
            const size_t seg_cnt = ray.segments.size();
            const auto distance = ray.hit ? ray.hit->distance : RAYCAST_MAX_DISTANCE_TILES;
            const auto exit_view = ray.segments.empty() ? raycast_result.default_view : ray.segments.back().view;
            handle_segment(ray, seg_cnt, exit_view, distance);

            for (int i = 0; i < ray.segments.size(); ++i) {
                const auto layer = seg_cnt + i + 1;
                const auto exit_view = (i == seg_cnt - 1) ? raycast_result.default_view : ray.segments[seg_cnt - i - 2].view;
                const auto distance = ray.segments[seg_cnt - i - 1].distance;
                handle_segment(
                    ray,
                    layer,
                    exit_view,
                    distance
                );
            }
        }

        unsigned int buffer_head = 0;
        for (auto &layer: layers_chunks) {
            for (auto& chunk: layer | std::views::values) {
                if (chunk.vertices.size() < 2) continue;

                // complete the fan shape by copying first vertex as last
                chunk.vertices.push_back(chunk.vertices[1]);
                vertex_count += 1;

                // update buffer_offset
                chunk.buffer_offset = buffer_head;
                buffer_head += chunk.vertices.size();
            }
        }
    }

    void ViewStencilState::_debug_colorize() {
        //HACK: for quick debugging
        constexpr std::array<sf::Color, 7> colors {
            sf::Color::Red,
            sf::Color::Green,
            sf::Color::Blue,
            sf::Color::Yellow,
            sf::Color::Magenta,
            sf::Color::Cyan,
            sf::Color::White
        };
        // assign each seen viewkey a sequential index
        std::unordered_map<ViewKey, size_t, ViewKeyHash> viewkey_indices{};
        size_t index = 0;
        for (const auto &layer: layers_chunks) {
            for (const auto &[viewkey, chunk]: layer) {
                if (viewkey_indices.find(viewkey) == viewkey_indices.end())
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
        for (const auto &layer: layers_chunks) {
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
