#include "render_view_pick.h"

#include <systems/resource_system.h>
#include <systems/shader_system.h>
#include <systems/texture_system.h>
#include <resources/ui_text.h>

namespace egkr::render_view
{
	render_view_pick::render_view_pick(const configuration& configuration)
		: render_view(configuration)
	{
		colour_target_attachment = texture::texture::create();
		depth_target_attachment = texture::texture::create();

		regenerate_render_targets();
	}

	bool render_view_pick::on_create()
	{
		world_shader_info.renderpass = renderpasses_[0];
		ui_shader_info.renderpass = renderpasses_[1];
		{
			std::string ui_shader_name = "Shader.Builtin.UIPickShader";
			auto resource = resource_system::load(ui_shader_name, resource_type::shader, nullptr);
			auto shader = (shader::properties*)resource->data;
			ui_shader_info.shader = shader_system::create_shader(*shader, ui_shader_info.renderpass.get());
			resource_system::unload(resource);

			ui_shader_info.id_colour_location = ui_shader_info.shader->get_uniform_index("id_colour");
			ui_shader_info.projection_location = ui_shader_info.shader->get_uniform_index("projection");
			ui_shader_info.view_location = ui_shader_info.shader->get_uniform_index("view");
			ui_shader_info.model_location = ui_shader_info.shader->get_uniform_index("model");
			ui_shader_info.near_clip = -100.F;
			ui_shader_info.far_clip = 100.F;
			ui_shader_info.fov = 0.F;
			ui_shader_info.projection = glm::ortho(0.F, (float)width_, (float)height_, 0.F, ui_shader_info.near_clip, ui_shader_info.far_clip);
			ui_shader_info.view = float4x4{ 1.F };
		}
		{
			const std::string world_shader_name = "Shader.Builtin.WorldPickShader";
			auto world_resource = resource_system::load(world_shader_name, resource_type::shader, nullptr);
			auto world_shader = (shader::properties*)world_resource->data;
			world_shader_info.shader = shader_system::create_shader(*world_shader, world_shader_info.renderpass.get());
			resource_system::unload(world_resource);

			world_shader_info.id_colour_location = world_shader_info.shader->get_uniform_index("id_colour");
			world_shader_info.projection_location = world_shader_info.shader->get_uniform_index("projection");
			world_shader_info.view_location = world_shader_info.shader->get_uniform_index("view");
			world_shader_info.model_location = world_shader_info.shader->get_uniform_index("model");
			world_shader_info.near_clip = 0.1F;
			world_shader_info.far_clip = 1000.F;
			world_shader_info.fov = glm::radians(45.F);
			world_shader_info.projection = glm::perspective(camera_->get_fov(), (float)width_ / height_, camera_->get_near_clip(), camera_->get_far_clip());
			world_shader_info.view = float4x4{ 1.F };
		}
		event::register_event(event_code::mouse_move, this, on_mouse_move);
		event::register_event(event_code::render_target_refresh_required, this, on_event);
			
			return true;
	}

	bool render_view_pick::on_destroy()
	{
		event::unregister_event(event_code::render_target_refresh_required, this, on_event);
		event::unregister_event(event_code::mouse_move, this, on_mouse_move);

		colour_target_attachment->destroy();
		depth_target_attachment->destroy();
		return false;
	}

	void render_view_pick::on_resize(uint32_t width, uint32_t height)
	{
		if (width != width_ || height != height_)
		{
			width_ = width;
			height_ = height;
			world_shader_info.projection = glm::perspective(camera_->get_fov(), (float)width_ / (float)height_, camera_->get_near_clip(), camera_->get_far_clip());
			ui_shader_info.projection = glm::ortho(0.F, (float)width_, (float)height_, 0.F, ui_shader_info.near_clip, ui_shader_info.far_clip);

			std::ranges::for_each(renderpasses_, [width, height](auto& pass) { pass->set_render_area(width, height); });
		}
	}

	render_view_packet render_view_pick::on_build_packet(void* data)
	{
		auto* packet_data = (pick_packet_data*)data;
		render_data = *packet_data;

		render_view_packet packet
		{
			.render_view = this
		};

		world_shader_info.view = camera_->get_view();
		ui_shader_info.view = camera_->get_view();

		uint32_t hightest_instance_id{};
		for (const auto& world_mesh : packet_data->world_mesh_data.meshes)
		{
			const auto unique_id = world_mesh->get_unique_id();
			for (const auto& geometry : world_mesh->get_geometries())
			{
				geometry::render_data geo
				{
					.geometry = geometry,
					.model = world_mesh->get_model(),
					.unique_id = unique_id
				};
				packet.render_data.push_back(geo);
			}

			if (unique_id > hightest_instance_id)
			{
				hightest_instance_id = unique_id;
			}
		}

		for (const auto& ui_mesh : packet_data->ui_mesh_data.meshes)
		{
			const auto unique_id = ui_mesh->get_unique_id();
			for (const auto& geometry : ui_mesh->get_geometries())
			{
				packet.render_data.emplace_back(geometry, ui_mesh->get_model(), unique_id);
			}

			if (unique_id > hightest_instance_id)
			{
				hightest_instance_id = unique_id;
			}
		}

		for (const auto& text_mesh : packet_data->texts)
		{
			const auto unique_id = text_mesh->get_unique_id();
			if (unique_id > hightest_instance_id)
			{
				hightest_instance_id = unique_id;
			}
		}

		uint32_t required_highest_instance = hightest_instance_id + 1;
		if (required_highest_instance > instance_updated.size())
		{
			uint32_t diff = required_highest_instance;
			for (auto i{ 0U }; i < diff; ++i)
			{
				acquire_shader_instances();
			}
		}
		return packet;
	}

	bool render_view_pick::on_render(const render_view_packet* render_view_packet, uint32_t /*frame_number*/, uint32_t render_target_index)
	{
		//Only do this every three frames
		if (render_target_index == 0)
		{
			uint32_t p{};
			renderpass::renderpass::shared_ptr pass = renderpasses_[p];

			std::ranges::fill(instance_updated, false);
			pass->begin(pass->get_render_target(render_target_index).get());


			shader_system::use(world_shader_info.shader->get_id());
			shader_system::set_uniform(world_shader_info.projection_location, &world_shader_info.projection);
			shader_system::set_uniform(world_shader_info.view_location, &world_shader_info.view);
			shader_system::apply_global();

			for (auto i{ 0U }; i < render_data.world_mesh_data.meshes.size(); ++i)
			{
				auto& render_data = render_view_packet->render_data[i];
				world_shader_info.shader->bind_instances(render_data.unique_id);

				const auto [r, g, b] = u32_to_rgb(render_data.unique_id);
				float3 id_colour = rgbu_to_float3(r, g, b);
				shader_system::set_uniform(world_shader_info.id_colour_location, &id_colour);

				bool needs_update = !instance_updated[render_data.unique_id];
				shader_system::apply_instance(needs_update);
				instance_updated[render_data.unique_id] = true;

				shader_system::set_uniform(world_shader_info.model_location, &render_data.model);
				render_data.geometry->draw();
			}
			pass->end();

			//p++;
			//pass = renderpasses_[p];

			//pass->begin(pass->get_render_target(render_target_index).get());
			//shader_system::use(ui_shader_info.shader->get_id());
			//shader_system::set_uniform(ui_shader_info.projection_location, &ui_shader_info.projection);
			//shader_system::set_uniform(ui_shader_info.view_location, &ui_shader_info.view);
			//shader_system::apply_global();

			//for (auto i{ render_data.world_mesh_data.meshes.size() }; i < render_view_packet->render_data.size(); ++i)
			//{
			//	auto& render_data = render_view_packet->render_data[i];
			//	ui_shader_info.shader->bind_instances(render_data.unique_id);

			//	const auto [r, g, b] = u32_to_rgb(render_data.unique_id);
			//	float3 id_colour = rgbu_to_float3(r, g, b);
			//	shader_system::set_uniform(ui_shader_info.id_colour_location, &id_colour);

			//	bool needs_update = !instance_updated[render_data.unique_id];
			//	shader_system::apply_instance(needs_update);
			//	instance_updated[render_data.unique_id] = true;

			//	shader_system::set_uniform(ui_shader_info.model_location, &render_data.model);
			//	render_data.geometry->draw();
			//}

			//for (auto& text : render_data.texts)
			//{
			//	const auto unique_id = text->get_unique_id();
			//	shader_system::bind_instance(unique_id);
			//	const auto [r, g, b] = u32_to_rgb(unique_id);
			//	float3 id_colour = rgbu_to_float3(r, g, b);
			//	shader_system::set_uniform(ui_shader_info.id_colour_location, &id_colour);

			//	bool needs_update = !instance_updated[unique_id];
			//	shader_system::apply_instance(needs_update);
			//	instance_updated[unique_id] = true;

			//	const auto& model = text->get_transform().get_world();
			//	shader_system::set_uniform(ui_shader_info.model_location, &model);
			//	text->draw();
			//}

			//pass->end();

			uint16_t x_coord = std::clamp((int32_t)mouse_x, 0, (int32_t)width_ - 1);
			uint16_t y_coord = std::clamp((int32_t)mouse_y, 0, (int32_t)height_ - 1);

			auto pixel = colour_target_attachment->read_pixel(x_coord, y_coord);
			uint32_t id = rgba_to_u32(pixel.r, pixel.g, pixel.b);

			event_context context{};
			context.context_ = std::array<uint32_t, 4> {id};
			event::fire_event(event_code::hover_id_changed, nullptr, context);
		}


		return true;
	}

	bool render_view_pick::regenerate_attachment_target(uint32_t pass_index, render_target::attachment& attachment)
	{
		if (attachment.type == render_target::attachment_type::colour)
		{
			attachment.texture = colour_target_attachment;
		}
		else if (attachment.type == render_target::attachment_type::depth)
		{
			attachment.texture = depth_target_attachment;
		}
		else
		{
			LOG_FATAL("Unrecognised attachment format");
			return false;
		}

//		if (pass_index == 1)
		{
		//	return true;
		}

		if (attachment.texture)
		{
			attachment.texture->destroy();
		}

		uint32_t width = renderpasses_[pass_index]->get_render_area().z;
		uint32_t height = renderpasses_[pass_index]->get_render_area().w;

		bool has_transparency{};

		texture::properties properties
		{
			.name = "pick_attachment",
			.id = invalid_32_id,
			.width = width,
			.height = height,
			.channel_count = 4,
			.flags = texture::flags::is_writable,
			.texture_type = texture::type::texture_2d
		};

		if (has_transparency)
		{
			properties.flags |= texture::flags::has_transparency;
		}

		if (attachment.type == render_target::attachment_type::depth)
		{
			properties.flags |= texture::flags::depth;
		}

		texture::texture::create(properties, nullptr, attachment.texture);
		attachment.texture->populate_writeable();
		return true;
	}

	void render_view_pick::acquire_shader_instances()
	{
		ui_shader_info.shader->acquire_instance_resources({});
		world_shader_info.shader->acquire_instance_resources({});

		instance_updated.push_back(false);
	}

	void render_view_pick::release_shader_instances()
	{
		for (auto i{ 0U }; i < instance_updated.size(); ++i)
		{
		}
	}

	bool render_view_pick::on_mouse_move(event_code code, void* /*sender*/, void* listener, const event_context& context)
	{
		if (code == event_code::mouse_move)
		{
			auto* view = (render_view_pick*)listener;
			context.get(0, view->mouse_x);
			context.get(1, view->mouse_y);

			return true;
		}
		return false;
	}
}
