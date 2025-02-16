#include "vulkan_shader.h"
#include "vulkan_types.h"

#include "systems/resource_system.h"
#include "systems/texture_system.h"

namespace egkr
{
	shader::shared_ptr vulkan_shader::create(const vulkan_context* context, const properties& shader_properties)
	{
		return std::make_shared<vulkan_shader>(context, shader_properties);
	}

	vulkan_shader::vulkan_shader(const vulkan_context* context, const properties& shader_properties) : shader(shader_properties), context_{ context }
	{
	}
	vulkan_shader::~vulkan_shader()
	{
		free();
	}

	void vulkan_shader::free()
	{
		context_->device.logical_device.waitIdle();
		if (uniform_buffer)
		{
			uniform_buffer.reset();
		}

		for (const auto& layout : descriptor_set_layout)
		{
			context_->device.logical_device.destroyDescriptorSetLayout(layout, context_->allocator);
		}
		descriptor_set_layout.clear();

		if (!global_descriptor_sets.empty())
		{
			context_->device.logical_device.freeDescriptorSets(descriptor_pool, global_descriptor_sets);
			global_descriptor_sets.clear();
		}

		for (auto& instance_state : instance_states)
		{
			for (const auto& set : instance_state.descriptor_set_state.descriptor_sets)
			{
				context_->device.logical_device.freeDescriptorSets(descriptor_pool, set);
			}
			instance_state.descriptor_set_state.descriptor_sets.clear();
		}
		instance_states.clear();

		if (descriptor_pool)
		{
			context_->device.logical_device.destroyDescriptorPool(descriptor_pool, context_->allocator);
			descriptor_pool = VK_NULL_HANDLE;
		}

		for (auto& pipeline : pipelines_)
		{
			if (pipeline)
			{
				pipeline->destroy();
			}
		}

		pipelines_.clear();

		for (const auto& stage : vulkan_stages)
		{
			context_->device.logical_device.destroyShaderModule(stage.handle, context_->allocator);
		}
		vulkan_stages.clear();
	}

	bool vulkan_shader::use()
	{
		const auto& command_buffer = context_->graphics_command_buffers[context_->image_index];
		pipelines_[bound_pipeline_index_]->bind(command_buffer, vk::PipelineBindPoint::eGraphics);
		command_buffer.get_handle().setPrimitiveTopology(current_topology_);
		return true;
	}

	bool vulkan_shader::populate(renderpass::renderpass* pass, const egkr::vector<std::string>& stage_filenames, const egkr::vector<stages>& shader_stages)
	{
		egkr::vector<vk::ShaderStageFlagBits> stage_flags{};
		for (const auto stage : shader_stages)
		{
			switch (stage)
			{
			case stages::fragment:
				stage_flags.push_back(vk::ShaderStageFlagBits::eFragment);
				break;
			case stages::vertex:
				stage_flags.push_back(vk::ShaderStageFlagBits::eVertex);
				break;
			case stages::geometry:
				stage_flags.push_back(vk::ShaderStageFlagBits::eGeometry);
				break;
			case stages::compute:
				stage_flags.push_back(vk::ShaderStageFlagBits::eCompute);
				break;
			default:
				LOG_ERROR("Unrecognised shader stage");
				return false;
			}
		}

		renderpass = (renderpass::vulkan_renderpass*)pass;
		configuration.max_descriptor_set_count = 1024;

		for (auto i{ 0U }; i < stage_flags.size(); ++i)
		{
			auto& stage = stage_flags[i];
			const auto& name = stage_filenames[i];
			configuration.stages.emplace_back(stage, name);
		}

		configuration.pool_sizes.push_back(vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 1024));
		configuration.pool_sizes.push_back(vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 4096));


		for (const auto& shader_uniform : uniforms_)
		{
			switch (shader_uniform.uniform_scope)
			{
			case scope::global:
			{
				if (shader_uniform.type == uniform_type::sampler)
				{
					properties_.global_uniform_sampler_count++;
				}
				else
				{
					properties_.global_uniform_count++;
				}
			}
			break;
			case scope::instance:
			{
				if (shader_uniform.type == uniform_type::sampler)
				{
					properties_.instance_uniform_sampler_count++;
				}
				else
				{
					properties_.instance_uniform_count++;
				}
			}
			break;
			case scope::local:
				properties_.local_uniform_count++;
				break;
			default:
				break;
			}
		}

		if (properties_.global_uniform_count || properties_.global_uniform_sampler_count)
		{
			vulkan_descriptor_set_configuration global_descriptor_set_configuration{};

			if (properties_.global_uniform_count)
			{
				auto binding_index = (uint32_t)configuration.descriptor_sets[DESCRIPTOR_SET_INDEX_GLOBAL].bindings.size();
				vk::DescriptorSetLayoutBinding ubo{};
				ubo.setBinding(binding_index).setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eUniformBuffer).setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);

				global_descriptor_set_configuration.bindings.push_back(ubo);
			}

			if (properties_.global_uniform_sampler_count)
			{
				auto binding_index = (uint8_t)configuration.descriptor_sets[DESCRIPTOR_SET_INDEX_GLOBAL].bindings.size();
				vk::DescriptorSetLayoutBinding ubo{};
				ubo.setBinding(binding_index)
					.setDescriptorCount(properties_.global_uniform_sampler_count)
					.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
					.setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);
				configuration.descriptor_sets[DESCRIPTOR_SET_INDEX_GLOBAL].sampler_binding_index = binding_index;
				global_descriptor_set_configuration.bindings.push_back(ubo);
			}
			configuration.descriptor_sets[DESCRIPTOR_SET_INDEX_GLOBAL] = global_descriptor_set_configuration;
		}

		if (has_instances())
		{
			vulkan_descriptor_set_configuration instance_descriptor_set_configuration{};
			if (properties_.instance_uniform_count)
			{
				auto binding_index = configuration.descriptor_sets[DESCRIPTOR_SET_INDEX_INSTANCE].binding_count;
				vk::DescriptorSetLayoutBinding ubo{};
				ubo.setBinding(binding_index).setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eUniformBuffer).setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);

				instance_descriptor_set_configuration.bindings.push_back(ubo);
				configuration.descriptor_sets[DESCRIPTOR_SET_INDEX_INSTANCE].binding_count++;
			}

			if (properties_.instance_uniform_sampler_count)
			{
				auto binding_index = (uint8_t)configuration.descriptor_sets[DESCRIPTOR_SET_INDEX_INSTANCE].binding_count;
				vk::DescriptorSetLayoutBinding ubo{};
				ubo.setBinding(binding_index)
					.setDescriptorCount(properties_.instance_uniform_sampler_count)
					.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
					.setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);

				instance_descriptor_set_configuration.bindings.push_back(ubo);
				instance_descriptor_set_configuration.sampler_binding_index = binding_index;
				instance_descriptor_set_configuration.binding_count++;
			}
			configuration.descriptor_sets[DESCRIPTOR_SET_INDEX_INSTANCE] = instance_descriptor_set_configuration;
		}

		for (auto i{ 0U }; i < stage_flags.size(); ++i)
		{
			vulkan_stages.push_back(create_module(configuration.stages[i]));
		}

		const auto& attributes = get_attributes();

		uint32_t offset{ 0 };
		for (auto i{ 0U }; i < attributes.size(); ++i)
		{
			vk::VertexInputAttributeDescription vertex_attribute{};
			vertex_attribute.setLocation(i).setBinding(0).setOffset(offset).setFormat(vulkan_attribute_types[attributes[i].type]);
			configuration.attributes.push_back(vertex_attribute);

			offset += attributes[i].size;
		}

		vk::DescriptorPoolCreateInfo pool_info{};
		pool_info.setPoolSizes(configuration.pool_sizes).setMaxSets(configuration.max_descriptor_set_count).setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

		descriptor_pool = context_->device.logical_device.createDescriptorPool(pool_info, context_->allocator);

		for (auto i{ 0U }; i < configuration.descriptor_sets.size(); i++)
		{
			vk::DescriptorSetLayoutCreateInfo layout_info{};
			layout_info.setBindings(configuration.descriptor_sets[i].bindings);

			descriptor_set_layout[i] = context_->device.logical_device.createDescriptorSetLayout(layout_info, context_->allocator);
		}

		vk::Viewport viewport{};
		viewport.setX(0.F).setY((float)context_->framebuffer_height).setWidth((float)context_->framebuffer_width).setHeight(-(float)context_->framebuffer_height).setMinDepth(0.F).setMaxDepth(1.F);

		vk::Rect2D scissor{};
		scissor.setExtent({ context_->framebuffer_width, context_->framebuffer_height }).setOffset({ 0, 0 });

		egkr::vector<vk::PipelineShaderStageCreateInfo> stage_create_infos{};
		for (const auto& stage : vulkan_stages)
		{
			stage_create_infos.push_back(stage.shader_stage_create_info);
		}

		vk::VertexInputBindingDescription binding_desc{};
		binding_desc.setBinding(0).setStride(get_attribute_stride()).setInputRate(vk::VertexInputRate::eVertex);

		pipeline_properties pipeline_properties{};
		pipeline_properties.renderpass = renderpass;
		pipeline_properties.descriptor_set_layout = descriptor_set_layout;
		pipeline_properties.shader_stage_info = stage_create_infos;
		pipeline_properties.is_wireframe = false;
		pipeline_properties.scissor = scissor;
		pipeline_properties.viewport = viewport;
		pipeline_properties.push_constant_ranges = get_push_constant_ranges();
		pipeline_properties.input_binding_description = binding_desc;
		pipeline_properties.input_attribute_description = configuration.attributes;
		pipeline_properties.cull_mode = properties_.shader_cull_mode;
		pipeline_properties.shader_name = properties_.name;
		pipeline_properties.shader_flags = properties_.shader_flags;
		bool pipeline_bound{};

		if (topology_types_ & primitive_topology_type::point_list)
		{
			pipeline_properties.topology_types = primitive_topology_type::point_list;
			pipelines_[(int)topology_class::point] = pipeline::create(context_, pipeline_properties);
			bound_pipeline_index_ = (int)topology_class::point;
			current_topology_ = vk::PrimitiveTopology::ePointList;
			pipeline_bound = true;
		}
		if (topology_types_ & primitive_topology_type::line_list || topology_types_ & primitive_topology_type::line_strip)
		{
			pipeline_properties.topology_types = primitive_topology_type::line_list | primitive_topology_type::line_strip;
			pipelines_[(int)topology_class::line] = pipeline::create(context_, pipeline_properties);
			bound_pipeline_index_ = (int)topology_class::line;
			current_topology_ = vk::PrimitiveTopology::eLineList;
			pipeline_bound = true;
		}
		if (topology_types_ & primitive_topology_type::triangle_list || topology_types_ & primitive_topology_type::triangle_strip || topology_types_ & primitive_topology_type::triangle_fan)
		{
			pipeline_properties.topology_types = primitive_topology_type::triangle_fan | primitive_topology_type::triangle_strip | primitive_topology_type::triangle_list;
			pipelines_[(int)topology_class::triangle] = pipeline::create(context_, pipeline_properties);
			bound_pipeline_index_ = (int)topology_class::triangle;
			current_topology_ = vk::PrimitiveTopology::eTriangleList;
			pipeline_bound = true;
		}


		if (!pipeline_bound)
		{
			LOG_ERROR("No supported pipeline found");
			return false;
		}

		set_global_ubo_stride(get_aligned(get_global_ubo_size(), 256));
		set_ubo_stride(get_aligned(get_ubo_size(), 256));

		uniform_buffer = renderbuffer::renderbuffer::create(renderbuffer::type::uniform, get_global_ubo_stride() + (get_ubo_stride() * max_material_count));
		uniform_buffer->bind(0);

		mapped_uniform_buffer_memory = uniform_buffer->map_memory(0, VK_WHOLE_SIZE);

		egkr::vector<vk::DescriptorSetLayout> global_layouts
			= { descriptor_set_layout[DESCRIPTOR_SET_INDEX_GLOBAL], descriptor_set_layout[DESCRIPTOR_SET_INDEX_GLOBAL], descriptor_set_layout[DESCRIPTOR_SET_INDEX_GLOBAL] };

		vk::DescriptorSetAllocateInfo alloc_info{};
		alloc_info.setDescriptorPool(descriptor_pool).setSetLayouts(global_layouts);

		global_descriptor_sets = context_->device.logical_device.allocateDescriptorSets(alloc_info);
		return false;
	}

	bool vulkan_shader::bind_instances(uint32_t instance_id)
	{
		set_bound_instance_id(instance_id);
		set_bound_ubo_offset(instance_states[instance_id].offset);
		return true;
	}

	bool vulkan_shader::apply_instances(bool needs_update)
	{
		if (!has_instances())
		{
			LOG_ERROR("This shader does not use instances");
			return false;
		}

		const auto image_index = context_->image_index;
		const auto& command_buffer = context_->graphics_command_buffers[image_index].get_handle();

		auto& object_state = instance_states[get_bound_instance_id()];
		auto& object_descriptor_set = object_state.descriptor_set_state.descriptor_sets[image_index];

		auto offset = object_state.offset;
		auto range = get_ubo_stride();
		vk::DescriptorBufferInfo buffer_info{};
		if (needs_update)
		{
			egkr::vector<vk::WriteDescriptorSet> writes{};
			uint32_t descriptor_count{};
			uint32_t descriptor_index{};
			if (properties_.instance_uniform_count)
			{
				auto& instance_ubo_generation = object_state.descriptor_set_state.descriptor_states[descriptor_index].generations[image_index];

				if (instance_ubo_generation == invalid_8_id)
				{
					buffer_info.setBuffer((*(vk::Buffer*)(uniform_buffer->get_buffer()))).setOffset(offset).setRange(range);

					vk::WriteDescriptorSet ubo{};
					ubo.setDstSet(object_descriptor_set).setDstBinding(descriptor_index).setDescriptorType(vk::DescriptorType::eUniformBuffer).setDescriptorCount(1).setBufferInfo(buffer_info);

					writes.push_back(ubo);
					descriptor_count++;

					instance_ubo_generation = 1;
				}
				++descriptor_index;
			}

			egkr::vector<vk::DescriptorImageInfo> image_infos{};
			vk::WriteDescriptorSet sampler{};
			if (properties_.instance_uniform_sampler_count)
			{
				auto sampler_binding_index = configuration.descriptor_sets[DESCRIPTOR_SET_INDEX_INSTANCE].sampler_binding_index;
				auto total_sampler_count = configuration.descriptor_sets[DESCRIPTOR_SET_INDEX_INSTANCE].bindings[sampler_binding_index].descriptorCount;
				{
					uint32_t update_sampler_count{};

					for (auto i{ 0U }; i < total_sampler_count; ++i)
					{
						auto& texture_map = instance_states[get_bound_instance_id()].instance_textures[i];

						auto vulkan_map = (vulkan_texture_map*)texture_map.get();

						auto texture_data = texture_map->map_texture;

						if (texture_data->get_generation() == invalid_32_id)
						{
							switch (vulkan_map->use)
							{
							case texture_map::use::map_diffuse:
								texture_data = texture_system::get_default_diffuse_texture();
								break;
							case texture_map::use::map_specular:
								texture_data = texture_system::get_default_specular_texture();
								break;
							case texture_map::use::map_normal:
								texture_data = texture_system::get_default_normal_texture();
								break;
							default:
								texture_data = texture_system::get_default_texture();
								break;
							}
						}
						else
						{
							if (texture_data->get_generation() != texture_map->get_generation())
							{
								if ((texture_data->get_mips() != texture_map->get_mips()) && !texture_map->refresh())
								{
									LOG_WARN("Couldn't refresh texture map resources");
								}
								else
								{
									texture_map->generation = texture_data->get_generation();
								}
							}
						}
						vk::DescriptorImageInfo image_info{};
						image_info.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal).setImageView(((vulkan_texture*)(texture_data.get()))->get_view()).setSampler(vulkan_map->sampler);

						image_infos.push_back(image_info);

						++update_sampler_count;
					}

					sampler.setDstSet(object_descriptor_set)
						.setDstBinding(descriptor_index)
						.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
						.setDescriptorCount(update_sampler_count)
						.setImageInfo(image_infos);

					writes.push_back(sampler);
					++descriptor_count;
				}
			}
			if (descriptor_count > 0)
			{
				context_->device.logical_device.updateDescriptorSets(writes, nullptr);
			}
		}
		command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelines_[bound_pipeline_index_]->get_layout(), 1, object_descriptor_set, nullptr);
		return true;
	}

	bool vulkan_shader::bind_globals()
	{
		set_bound_ubo_offset(get_global_ubo_offset());
		return true;
	}

	bool vulkan_shader::apply_globals(bool needs_update)
	{
		const auto image_index = context_->image_index;
		auto& global_descriptor = global_descriptor_sets[image_index];
		if (needs_update)
		{

			vk::DescriptorBufferInfo buffer_info{};
			buffer_info.setBuffer(*(vk::Buffer*)(uniform_buffer->get_buffer())).setOffset(get_global_ubo_offset()).setRange(get_global_ubo_stride());

			vk::WriteDescriptorSet ubo_write{};
			ubo_write.setDstSet(global_descriptor).setDstBinding(0).setDstArrayElement(0).setDescriptorType(vk::DescriptorType::eUniformBuffer).setDescriptorCount(1).setBufferInfo(buffer_info);

			egkr::vector<vk::WriteDescriptorSet> writes{ ubo_write };

			context_->device.logical_device.updateDescriptorSets(writes, nullptr);
		}
		const auto& command_buffer = context_->graphics_command_buffers[image_index].get_handle();
		command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelines_[bound_pipeline_index_]->get_layout(), 0, 1, &global_descriptor, 0, nullptr);

		return true;
	}

	uint32_t vulkan_shader::acquire_instance_resources(const egkr::vector<texture_map::shared_ptr>& texture_maps)
	{
		auto instance_id = (uint32_t)instance_states.size();

		vulkan_instance_state instance_state;

		instance_state.instance_textures = texture_maps;
		instance_state.offset = get_global_ubo_stride();

		egkr::vector<vk::DescriptorSetLayout> layouts{
			descriptor_set_layout[DESCRIPTOR_SET_INDEX_INSTANCE], descriptor_set_layout[DESCRIPTOR_SET_INDEX_INSTANCE], descriptor_set_layout[DESCRIPTOR_SET_INDEX_INSTANCE] };

		vk::DescriptorSetAllocateInfo alloc_info{};
		alloc_info.setDescriptorPool(descriptor_pool).setSetLayouts(layouts);

		instance_state.descriptor_set_state.descriptor_sets = context_->device.logical_device.allocateDescriptorSets(alloc_info);

		instance_states.push_back(instance_state);
		return instance_id;
	}

	bool vulkan_shader::set_uniform(const uniform& shader_uniform, const void* value)
	{
		if (shader_uniform.type == uniform_type::sampler)
		{
			if (shader_uniform.uniform_scope == scope::global)
			{
				set_global_texture(shader_uniform.location, ((const texture_map::shared_ptr*)(value))->get());
			}
			else
			{
				instance_states[get_bound_instance_id()].instance_textures[shader_uniform.location] = *(const texture_map::shared_ptr*)value;
			}
		}
		else
		{
			if (shader_uniform.uniform_scope == scope::local)
			{
				// Is local, using push constants. Do this immediately.
				const auto& command_buffer = context_->graphics_command_buffers[context_->image_index].get_handle();
				command_buffer.pushConstants(
					pipelines_[bound_pipeline_index_]->get_layout(), vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, (uint32_t)shader_uniform.offset, shader_uniform.size, value);
			}
			else
			{
				// Map the appropriate memory location and copy the data over.
				auto addr = (uint8_t*)mapped_uniform_buffer_memory;
				addr += get_bound_ubo_offset() + shader_uniform.offset;

				memcpy(addr, value, shader_uniform.size);
			}
		}
		return true;
	}

	vulkan_stage vulkan_shader::create_module(const vulkan_stage_configuration& stage_configuration)
	{
		auto shader_resource = resource_system::load(stage_configuration.filename, resource::type::binary, nullptr);
		auto* code = (binary_resource_properties*)shader_resource->data;

		vulkan_stage shader_stage{};

		vk::ShaderModuleCreateInfo create_info{};
		create_info.setCodeSize(code->data.size()).setPCode((uint32_t*)(code->data.data()));
		shader_stage.handle = context_->device.logical_device.createShaderModule(create_info, context_->allocator);
		shader_stage.create_info = create_info;

		resource_system::unload(shader_resource);

		vk::PipelineShaderStageCreateInfo pipeline_create_info{};
		pipeline_create_info.setStage(stage_configuration.stage).setModule(shader_stage.handle).setPName("main");

		shader_stage.shader_stage_create_info = pipeline_create_info;

		return shader_stage;
	}
}
