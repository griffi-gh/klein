#include "klein/view/view_stencil.hpp"

#include <bit>
#include <ranges>
#include <stdexcept>
#include <unordered_map>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/StencilMode.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/System/Vector2.hpp>
#include <spdlog/spdlog.h>

#include "klein/tilemap/tilemap.hpp"
#include "klein/view/view_raycast.hpp"
#include "klein/view/raycast.hpp"

using std::views::zip, std::views::iota;

constexpr float EPSILON = 1e-6f;

namespace klein::view {
    static void simplify_mesh(std::vector<sf::Vertex>& vtx) {
        static std::vector<sf::Vertex> temp; // (reused)

        temp.clear();
        temp.reserve(vtx.size());

        for (const auto& v : vtx) {
            // exact duplicate
            if (!temp.empty() && temp.back().position == v.position) continue;
            temp.push_back(v);

            // collapse collinear triple at the LAST vtx
            while (temp.size() >= 3) {
                const auto& a = temp[temp.size() - 3].position;
                const auto& b = temp[temp.size() - 2].position;
                const auto& c = temp[temp.size() - 1].position;
                if (std::abs((b - a).cross(c - a)) > EPSILON) break;
                temp[temp.size() - 2] = temp.back();
                temp.pop_back();
            }
        }

        vtx.swap(temp);
    }

    void ViewStencilState::update_staging(const RaycastViewResponse &raycast) {
        // TODO: do sth clever here (we could prob reuse most of the stuff in there)
        const size_t layer_count = (raycast.max_segments_depth + 1) * 2;
        if (layers_chunks.size() != layer_count) {
            layers_chunks.resize(layer_count);
        }
        for (auto& layer: layers_chunks) layer.clear();

        auto process_ray = [&] (const size_t ray_idx, const RayPath& ray) {
            const size_t layer_cnt = ray.segments.size() + 1;
            for (int layer = 0; layer < layer_cnt; ++layer) {
                const auto view =
                    layer == 0 ?
                    raycast.default_view :
                    ray.segments[layer - 1].view;
                const auto distance =
                    (layer == layer_cnt - 1) ?
                    (ray.hit ? ray.hit->distance : RAYCAST_MAX_DISTANCE_TILES):
                    ray.segments[layer].distance;

                auto &layer_ref = layers_chunks[layer];
                auto [it, inserted] = layer_ref.try_emplace(view);
                auto &chunk = it->second;

                if (inserted) {
                    chunk.stencil_value = raycast.views.at(view).stencil_idx;
                }

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

                // const auto &next_ray = raycast_result.rays[(ray_idx + 1) % raycast_result.rays.size()];
                // sf::Vector2f segment_pos_next = ray.origin_t + next_ray.direction * distance;
                // sf::Vector2f segment_pos_next_s = segment_pos_next.componentWiseMul(tilemap::TILE_SCREEN_SIZE);
                // chunk.vertices.push_back(sf::Vertex(segment_pos_next_s));

                chunk._last_ray_idx = ray_idx;
            }
        };

        for (const auto &[ray_idx, ray]: zip(iota(0uz), raycast.rays)) {
            process_ray(ray_idx, ray);
        }
        // sneakily pretent first ray is also the non-existend last one
        // this is needed so that shapes we get are pproperly closed off
        process_ray(raycast.rays.size(), raycast.rays[0]);

        // reset vertex_count
        vertex_count = 0;
        unsigned int buffer_head = 0;
        for (auto &layer: layers_chunks) {
            for (auto& chunk: layer | std::views::values) {
                if (chunk.vertices.size() < 2) continue;

                // "close off" last chunk
                chunk.vertices.push_back(chunk.vertices[0]);

                simplify_mesh(chunk.vertices);

                // mesh nuked after simplification - skip
                if (chunk.vertices.size() < 3) continue;

                // update vertex_count
                vertex_count += chunk.vertices.size();

                // update buffer_offset
                chunk.buffer_offset = buffer_head;
                buffer_head += static_cast<unsigned int>(chunk.vertices.size());
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
        size_t current_size = buffer.getVertexCount();
        size_t desired_size = vertex_count ? std::bit_ceil(vertex_count) : 0;
        if (current_size < desired_size) {
            spdlog::debug("ViewStencilState: growing vertex buffer {} -> {} vertices", current_size, desired_size);
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

    void ViewStencilState::draw_debug(sf::RenderTarget &target) const {
        for (const auto &layer: layers_chunks | std::views::reverse) {
            for (const auto& chunk: layer | std::views::values) {
                if (chunk.vertices.size() < 3) continue;
                target.draw(buffer, chunk.buffer_offset, chunk.vertices.size());
            }
        }
    }

    // draw the stuff to stencil only
    void ViewStencilState::draw_stencil(sf::RenderTarget &target, const RaycastViewResponse &raycast) const {
        target.clearStencil(sf::StencilValue(0xff));

        sf::RenderStates state;
        state.stencilMode = sf::StencilMode {
            .stencilUpdateOperation = sf::StencilUpdateOperation::Replace,
            .stencilOnly = true,
        };

        for (const auto &layer: layers_chunks | std::views::reverse) {
            for (const auto& [view, chunk]: layer) {
                if (chunk.vertices.size() < 3) continue;

                const unsigned int vk_reference = raycast.views.at(view).stencil_idx;
                state.stencilMode.stencilReference = sf::StencilValue(vk_reference);
                target.draw(buffer, chunk.buffer_offset, chunk.vertices.size(), state);
            }
        }
    }

}
