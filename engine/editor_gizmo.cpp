#include "editor_gizmo.h"
#include "identifier.h"
namespace egkr::editor
{
	static float scale_factor;
	constexpr uint8_t segment_count{ 32 };

	gizmo gizmo::create()
	{
		return gizmo();
	}

	gizmo::gizmo()
	{
	}

	gizmo::~gizmo()
	{
		unload();
		destroy();
	}

	void gizmo::init()
	{
		unique_id = identifier::acquire_unique_id(this);
		setup_none();
		setup_move();
		setup_rotate();
		setup_scale();
	}

	void gizmo::destroy()
	{
		for (const auto& data : mode_data | std::views::values)
		{
			data.geometry_->destroy();
		}
		mode_data.clear();
		identifier::release_id(unique_id);
		unique_id = invalid_32_id;
	}

	void gizmo::load()
	{
		for (auto& data : mode_data | std::views::values)
		{
			geometry::properties properties
			{
				.vertex_size = sizeof(colour_vertex_3d),
				.vertex_count = (uint32_t)data.vertices.size(),
				.vertices = (void*)data.vertices.data(),

				.indices = data.indices
			};
			data.geometry_ = geometry::create(properties);
			data.geometry_->upload();
		}
	}

	void gizmo::unload()
	{
	}

	void gizmo::update()
	{
	}

	void gizmo::set_mode(mode mode)
	{
		gizmo_mode = mode;
	}

	ray::result gizmo::raycast(const ray& ray)
	{
		ray::result result;
		const auto& gizmo_data = mode_data.at(gizmo_mode);

		for (const auto& extents : gizmo_data.extents)
		{
			ray.oriented_extents(extents, get_local())
				.transform([&](float distance)
						  {
							  ray::hit hit
							  {
								  .type = ray::hit_type::bounding_box,
								  .unique_id = unique_id,
								  .position = ray.origin + ray.direction * distance,
								  .distance = distance,
							  };
							  result.hits.push_back(hit);
							  return distance;
						  });
		}
		return result;
	}

	void gizmo::begin_interaction(interaction_type type, const ray& ray)
	{
		type_ = type;
		if (gizmo_mode == mode::move || gizmo_mode == mode::scale)
		{
			if (type == interaction_type::mouse_drag)
			{
				auto& data = mode_data[gizmo_mode];
				auto model = get_local();

				const float3 origin{ get_position() };

				switch (data.current_axis_index)
				{
				case 0:
				case 5:
					data.interaction_plane = plane::create(origin, model * float4{ 0.f, 1.f, 0.f, 0.f });
					break;
				case 1:
				case 3:
					data.interaction_plane = plane::create(origin, model * float4{ 0.f, 0.f, 1.f, 0.f });
					break;
				case 2:
				case 4:
					data.interaction_plane = plane::create(origin, model * float4{ 1.f, 0.f, 0.f, 0.f });
					break;
				default:
					break;
				}
				ray.plane(data.interaction_plane)
					.and_then([&](auto hit) ->std::optional<std::tuple<float3, float>>
							  {
								  auto& [point, dist] = hit;
								  data.interaction_start = data.last_interaction_point = point;
								  return hit;
							  });
			}
		}
	}

	void gizmo::end_interaction(interaction_type /*type*/, const ray& /*ray*/)
	{
		type_ = interaction_type::none;
		auto& data = mode_data.at(gizmo_mode);
		data.interaction_plane = {};
	}

	void gizmo::handle_interaction(interaction_type interaction_type, const ray& ray)
	{
		if (gizmo_mode == mode::move)
		{
			auto& data = mode_data.at(mode::move);
			if (interaction_type == interaction_type::mouse_hover)
			{
				float4x4 model;
				if (auto selected = selected_transform.lock())
				{
					model = selected->get_world();
				}
				uint8_t hit_axis{ invalid_8_id };

				float4x4 scale = glm::scale(glm::mat4(1.f), glm::vec3(scale_factor));
				model *= scale;

				for (auto i : std::ranges::iota_view( 0ull, data.extents.size() ) | std::views::reverse)
				{
					if (ray.oriented_extents(data.extents[(uint8_t)i], model))
					{
						hit_axis = (uint8_t)i;
						break;
					}
				}

				if (data.current_axis_index != hit_axis)
				{
					const float4 yellow{ 1.f, 1.f, 0.f, 1.f };
					data.current_axis_index = (uint8_t)hit_axis;

					for (uint8_t j{ 0u }; j < 3; j++)
					{
						if (j == hit_axis)
						{
							data.vertices[2 * j + 0].colour = yellow;
							data.vertices[2 * j + 1].colour = yellow;
						}
						else
						{
							data.vertices[2 * j + 0].colour = float4(0.f, 0.f, 0.f, 1.f);
							data.vertices[2 * j + 0].colour[j] = 1.f;
							data.vertices[2 * j + 1].colour = float4(0.f, 0.f, 0.f, 1.f);
							data.vertices[2 * j + 1].colour[j] = 1.f;
						}
					}
					if (hit_axis == 3)
					{
						data.vertices[6].colour = yellow;
						data.vertices[7].colour = yellow;
						data.vertices[10].colour = yellow;
						data.vertices[11].colour = yellow;
					}
					else
					{
						data.vertices[6].colour = float4{ 1.f, 0.f, 0.f, 1.f };
						data.vertices[7].colour = float4{ 1.f, 0.f, 0.f, 1.f };
						data.vertices[10].colour = float4{ 0.f, 1.f, 0.f, 1.f };
						data.vertices[11].colour = float4{ 0.f, 1.f, 0.f, 1.f };
					}

					if (hit_axis == 4)
					{
						data.vertices[12].colour = yellow;
						data.vertices[13].colour = yellow;
						data.vertices[16].colour = yellow;
						data.vertices[17].colour = yellow;
					}
					else
					{
						data.vertices[12].colour = float4{ 0.f, 1.f, 0.f, 1.f };
						data.vertices[13].colour = float4{ 0.f, 1.f, 0.f, 1.f };
						data.vertices[16].colour = float4{ 0.f, 0.f, 1.f, 1.f };
						data.vertices[17].colour = float4{ 0.f, 0.f, 1.f, 1.f };
					}

					if (hit_axis == 5)
					{
						data.vertices[14].colour = yellow;
						data.vertices[15].colour = yellow;
						data.vertices[8].colour = yellow;
						data.vertices[9].colour = yellow;
					}
					else
					{
						data.vertices[14].colour = float4{ 0.f, 0.f, 1.f, 1.f };
						data.vertices[15].colour = float4{ 0.f, 0.f, 1.f, 1.f };
						data.vertices[8].colour = float4{ 1.f, 0.f, 0.f, 1.f };
						data.vertices[9].colour = float4{ 1.f, 0.f, 0.f, 1.f };
					}

					if (hit_axis == 6)
					{
						std::ranges::for_each(data.vertices, [&](colour_vertex_3d& vert) { vert.colour = yellow; });
					}
					data.geometry_->update_vertices(0, (uint32_t)data.vertices.size(), data.vertices.data());
				}
			}
			else if (interaction_type == interaction_type::mouse_drag)
			{
				ray.plane(data.interaction_plane)
					.and_then([&](auto hit) -> std::optional<std::tuple<float3, float>>
								{
									auto& [point, dist] = hit;
									float3 diff = point - data.last_interaction_point;
									float3 translation;
									float3 direction{ 0.f };
									switch (data.current_axis_index)
									{
									case 0:
										direction = float3{ 1.f, 0.f, 0.f };
										translation = glm::dot(diff, direction) * direction;
										break;
									case 1:
										direction = float3{ 0.f, 1.f, 0.f };
										translation = glm::dot(diff, direction) * direction;
										break;
									case 2:
										direction = float3{ 0.f, 0.f, 1.f };
										translation = glm::dot(diff, direction) * direction;
										break;
									case 3:
									case 4:
									case 5:
									case 6:
										translation = diff;
										break;
									default:
										break;
									}
									if (auto t = selected_transform.lock())
									{
										t->translate(translation);
										data.last_interaction_point = point;
										LOG_INFO("Last point: {}\n", glm::to_string(point));

									}
									return hit;
								});
				
			}
		}
		else if (gizmo_mode == mode::scale)
		{
			auto& data = mode_data.at(mode::scale);
			if (interaction_type == interaction_type::mouse_hover)
			{
				float4x4 model;
				if (auto selected = selected_transform.lock())
				{
					model = selected->get_world();
				}
				uint8_t hit_axis{ invalid_8_id };

				float4x4 scale = glm::scale(glm::mat4(1.f), glm::vec3(scale_factor));
				model *= scale;

				for (auto i : std::ranges::iota_view(0ull, data.extents.size()) | std::views::reverse)
				{
					if (ray.oriented_extents(data.extents[(uint8_t)i], model))
					{
						hit_axis = (uint8_t)i;
						break;
					}
				}

				if (data.current_axis_index != hit_axis)
				{
					const float4 yellow{ 1.f, 1.f, 0.f, 1.f };
					data.current_axis_index = (uint8_t)hit_axis;
					{
						for (uint8_t j{ 0u }; j < 3; j++)
						{
							if (j == hit_axis)
							{
								data.vertices[2 * j + 0].colour = yellow;
								data.vertices[2 * j + 1].colour = yellow;
							}
							else
							{
								data.vertices[2 * j + 0].colour = float4(0.f, 0.f, 0.f, 1.f);
								data.vertices[2 * j + 0].colour[j] = 1.f;
								data.vertices[2 * j + 1].colour = float4(0.f, 0.f, 0.f, 1.f);
								data.vertices[2 * j + 1].colour[j] = 1.f;
							}
						}

						if (hit_axis == 3)
						{
							std::ranges::for_each(data.vertices, [&](colour_vertex_3d& vert) { vert.colour = yellow; });
						}
						else
						{
							data.vertices[6].colour = { 1.f, 0.f, 0.f, 1.f };
							data.vertices[7].colour = { 1.f, 0.f, 0.f, 1.f };
							data.vertices[8].colour = { 0.f, 1.f, 0.f, 1.f };
							data.vertices[9].colour = { 0.f, 1.f, 0.f, 1.f };
							data.vertices[10].colour = { 0.f, 0.f, 1.f, 1.f };
							data.vertices[11].colour = { 0.f, 0.f, 1.f, 1.f };
						}

					}
					data.geometry_->update_vertices(0, (uint32_t)data.vertices.size(), data.vertices.data());

				}
			}
			else if (interaction_type == interaction_type::mouse_drag)
			{
				ray.plane(data.interaction_plane)
					.and_then([&](auto hit) -> std::optional<std::tuple<float3, float>>
							  {
								  auto& [point, dist] = hit;
								  float3 diff = point - data.last_interaction_point;
								  float3 translation;
								  float3 direction{ 0.f };
								  switch (data.current_axis_index)
								  {
								  case 0:
									  direction = float3{ 1.f, 0.f, 0.f };
									  translation = glm::dot(diff, direction) * direction;
									  break;
								  case 1:
									  direction = float3{ 0.f, 1.f, 0.f };
									  translation = glm::dot(diff, direction) * direction;
									  break;
								  case 2:
									  direction = float3{ 0.f, 0.f, 1.f };
									  translation = glm::dot(diff, direction) * direction;
									  break;
								  case 3:
								  case 4:
								  case 5:
								  case 6:
									  translation = diff;
									  break;
								  default:
									  break;
								  }
								  if (auto t = selected_transform.lock())
								  {
									  t->set_scale(t->get_scale() + translation);
									  data.last_interaction_point = point;
									  LOG_INFO("Last point: {}\n", glm::to_string(point));

								  }
								  return hit;
							  });

			}
		}
		else if (gizmo_mode == mode::rotate)
		{
			auto& data = mode_data.at(mode::rotate);
			if (interaction_type == interaction_type::mouse_hover)
			{
				float4x4 model;
				if (auto selected = selected_transform.lock())
				{
					model = selected->get_world();
				}
				uint8_t hit_axis{ invalid_8_id };

				float4x4 scale = glm::scale(glm::mat4(1.f), glm::vec3(scale_factor));
				model *= scale;

				for (uint32_t i : std::ranges::iota_view{ 0u, 3u })
				{
					float4 normal{ 0.f, 0.f, 0.f, 1.f };
					normal[(int32_t)i] = 1.f;
					plane p = plane::create(get_position(), model * normal);
					if (ray.disk(p, get_position(), scale_factor - 0.1f, scale_factor + 0.1f))
					{
						hit_axis = (uint8_t)i;
						break;
					}

					//Might be on the back side of the plane
					p = plane::create(get_position(), model * -normal);
					if (ray.disk(p, get_position(), scale_factor - 0.1f, scale_factor + 0.1f))
					{
						hit_axis = (uint8_t)i;
						break;
					}

				}

				if (data.current_axis_index != hit_axis)
				{
					data.current_axis_index = (uint8_t)hit_axis;

					for (uint32_t i : std::ranges::iota_view{ 0u, 2u * 3u * segment_count })
					{
						float4 col{ 0.f, 0.f, 0.f, 1.f };
						col[i / (2 * segment_count)] = 1.f;
						data.vertices[i].colour = col;
					}

					if (hit_axis != invalid_8_id)
					{
						const float4 yellow{ 1.f, 1.f, 0.f, 1.f };

						for (uint32_t i : std::ranges::iota_view{ 2u * hit_axis * segment_count, 2u * (hit_axis + 1) * segment_count })
						{
							data.vertices[i].colour = yellow;
						}
					}
					data.geometry_->update_vertices(0, (uint32_t)data.vertices.size(), data.vertices.data());
				}
			}
		}
	}
	
	void gizmo::set_selected(const std::weak_ptr<transformable>& transform)
	{
		selected_transform = transform;
	}

	std::optional<float4x4> gizmo::get_selected_model() const
	{
		if (auto selected = selected_transform.lock())
		{
			return selected->get_world();
		}

		return {};
	}

	void gizmo::set_scale(float scale)
	{
		scale_factor = scale;
	}

	void gizmo::setup_none()
	{
		constexpr float4 gray{ 0.6f, 0.6f, 0.6f, 1.f };
		constexpr float axis_length{ 1.f };
		egkr::vector<colour_vertex_3d> vertices(6);
		vertices[0] = { .position = {0.F, 0.F, 0.F, 1.F}, .colour = gray };
		vertices[1] = { .position = {axis_length, 0.F, 0.F, 1.F}, .colour = gray };
		vertices[2] = { .position = {0.F, 0.F, 0.F, 1.F}, .colour = gray };
		vertices[3] = { .position = {0.F, axis_length, 0.F, 1.F}, .colour = gray };
		vertices[4] = { .position = {0.F, 0.F, 0.F, 1.F}, .colour = gray };
		vertices[5] = { .position = {0.F, 0.F, axis_length, 1.F}, .colour = gray };

		mode_data[gizmo::mode::none] = { .vertices = vertices };
	}

	void gizmo::setup_move()
	{
		egkr::vector<colour_vertex_3d> vertices(18);
		float axis_length{ 2.f };

		for (uint32_t i{ 0u }; i < 6; i += 2)
		{
			float4 position{ 0.f, 0.f, 0.f, 1.f };
			float4 colour{0.f, 0.f, 0.f, 1.f};
			position[i / 2] = 0.2f;
			colour[i / 2] = 1.f;
			vertices[i] = { .position = position, .colour = colour };
			position[i / 2] = axis_length;
			colour[i / 2] = 1.f;
			vertices[i + 1] = { .position = position, .colour = colour };
		}
		axis_length = 0.4f;
		float4 colour{ 1.f, 0.f, 0.f, 1.f };
		vertices[6] = { .position = {axis_length, 0.f, 0.f, 1.f}, .colour = colour };
		vertices[7] = { .position = {axis_length, axis_length, 0.f, 1.f}, .colour = colour };

		vertices[8] = { .position = {axis_length, 0.f, 0.f, 1.f}, .colour = colour };
		vertices[9] = { .position = {axis_length, 0.f, axis_length, 1.f}, .colour = colour };
		
		colour = { 0.f, 1.f, 0.f, 1.f };
		vertices[10] = { .position = {0.f, axis_length, 0.f, 1.f}, .colour = colour  };
		vertices[11] = { .position = {axis_length, axis_length, 0.f, 1.f}, .colour = colour };

		vertices[12] = { .position = {0.f, axis_length, 0.f, 1.f}, .colour = colour };
		vertices[13] = { .position = {0.f, axis_length, axis_length, 1.f}, .colour = colour };
		
		colour = { 0.f, 0.f, 1.f, 1.f };
		vertices[14] = { .position = {0.f, 0.f, axis_length, 1.f}, .colour = colour  };
		vertices[15] = { .position = {axis_length, 0.f, axis_length, 1.f}, .colour = colour };

		vertices[16] = { .position = {0.f, 0.f, axis_length, 1.f}, .colour = colour };
		vertices[17] = { .position = {0.f, axis_length, axis_length, 1.f}, .colour = colour };

		const float small_box{ 0.1f };
		extent3d x_box{ .min = {axis_length, -small_box, -small_box}, .max = {2.1f, small_box, small_box} };
		extent3d y_box{ .min = {-small_box, axis_length, -small_box}, .max = {small_box, 2.1f, small_box} };
		extent3d z_box{ .min = {-small_box, -small_box, axis_length}, .max = {small_box, small_box, 2.1f} };
		extent3d xy_box{ .min = {0.f, 0.f, 0.f}, .max = {axis_length, axis_length, 0.1} };
		extent3d yz_box{ .min = {0.f, 0.f, 0.f}, .max = {0.1f, axis_length, axis_length} };
		extent3d zx_box{ .min = {0.f, 0.f, 0.f}, .max = {axis_length, 0.1f, axis_length} };
		extent3d xyz_box{ .min = {-0.1f, -0.1, -0.1f}, .max = {0.1f, 0.1f, 0.1f} };
		egkr::vector<extent3d> extents{ x_box, y_box, z_box, xy_box, yz_box, zx_box, xyz_box };
		mode_data[gizmo::mode::move] = { .vertices = vertices,  .extents = extents};
	}

	void gizmo::setup_scale()
	{
		egkr::vector<colour_vertex_3d> vertices(12);
		float axis_length{ 2.f };

		for (uint32_t i{ 0u }; i < 6; i += 2)
		{
			float4 position{ 0.f, 0.f, 0.f, 1.f };
			float4 colour{ 0.f, 0.f, 0.f, 1.f };
			position[i / 2] = 0.2f;
			colour[i / 2] = 1.f;
			vertices[i] = { .position = position, .colour = colour };
			position[i / 2] = axis_length;
			colour[i / 2] = 1.f;
			vertices[i + 1] = { .position = position, .colour = colour };
		}
		axis_length = 0.4f;
		const float4 red{ 1.f, 0.f, 0.f, 1.f };
		const float4 green{ 0.f, 1.f, 0.f, 1.f };
		const float4 blue{ 0.f, 0.f, 1.f, 1.f };

		vertices[6] = { .position = {axis_length, 0.f, 0.f, 1.f}, .colour = red };
		vertices[7] = { .position = {0, axis_length, 0.f, 1.f}, .colour = green };

		vertices[8] = { .position = {0.f, axis_length, 0.f, 1.f}, .colour = green };
		vertices[9] = { .position = {0.f, 0.f, axis_length, 1.f}, .colour = blue };

		vertices[10] = { .position = {0.f, 0.f, axis_length, 1.f}, .colour = blue };
		vertices[11] = { .position = {axis_length, 0.f, 0.f, 1.f}, .colour = red };

		const float small_box{ 0.1f };
		extent3d x_box{ .min = {axis_length, -small_box, -small_box}, .max = {2.1f, small_box, small_box} };
		extent3d y_box{ .min = {-small_box, axis_length, -small_box}, .max = {small_box, 2.1f, small_box} };
		extent3d z_box{ .min = {-small_box, -small_box, axis_length}, .max = {small_box, small_box, 2.1f} };
		extent3d xyz_box{ .min = {-0.1f, -0.1, -0.1f}, .max = {0.1f, 0.1f, 0.1f} };
		egkr::vector<extent3d> extents{ x_box, y_box, z_box, xyz_box };
		mode_data[gizmo::mode::scale] = { .vertices = vertices,  .extents = extents };
	}

	void gizmo::setup_rotate()
	{
		constexpr int16_t vertex_count{ 3 * 2 * segment_count };
		egkr::vector<colour_vertex_3d> vertices;
		vertices.reserve(vertex_count);
		const float axis_length{ 1.f };
		constexpr const float two_pi{ 2.f * std::numbers::pi_v<float> };

		float4 colour{ 1.f, 0.f, 0.f, 1.f };
		for (uint8_t i{ 0u }; i < segment_count; ++i)
		{
			float4 position{ 0.f, std::sinf(axis_length * i * two_pi / segment_count), std::cosf(axis_length * i * two_pi / segment_count), 1.f };
			vertices.emplace_back(position, colour);

			position = { 0.f, std::sinf(axis_length * (i + 1) * two_pi / segment_count), std::cosf(axis_length * (i + 1) * two_pi / segment_count), 1.f };
			vertices.emplace_back(position, colour);
		}

		colour = { 0.f, 1.f, 0.f, 1.f };
		for (uint8_t i{ 0u }; i < segment_count; ++i)
		{
			float4 position{ std::sinf(axis_length * i * two_pi / segment_count), 0.f, std::cosf(axis_length * i * two_pi / segment_count), 1.f };
			vertices.emplace_back(position, colour);

			position = { std::sinf(axis_length * (i + 1) * two_pi / segment_count), 0.f, std::cosf(axis_length * (i + 1) * two_pi / segment_count), 1.f };
			vertices.emplace_back(position, colour);
		}

		colour = { 0.f, 0.f, 1.f, 1.f };
		for (uint8_t i{ 0u }; i < segment_count; ++i)
		{
			float4 position{ std::cosf(axis_length * i * two_pi / segment_count), std::sinf(axis_length * i * two_pi / segment_count), 0.f, 1.f };
			vertices.emplace_back(position, colour);

			position = { std::cosf(axis_length * (i + 1) * two_pi / segment_count), std::sinf(axis_length * (i + 1) * two_pi / segment_count), 0.f, 1.f };
			vertices.emplace_back(position, colour);
		}

		mode_data[gizmo::mode::rotate] = { .vertices = vertices };
	}
}
