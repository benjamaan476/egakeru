#include "shader.h"
#include <format>
#include <ranges>

#include "platform/filesystem.h"
#include "vulkan_types.h"
#include "vulkan_texture.h"

#include "systems/texture_system.h"

namespace egkr
{

	static std::unordered_map<shader_stages, std::string> stage_by_shader_stages = {
		{shader_stages::frag, "frag"},
		{shader_stages::vert, "vert"}
	};

	std::string shader_stage_to_string(shader_stages stage)
	{
		return stage_by_shader_stages[stage];
	}

	shader_stage shader::create_shader_module(const vulkan_context* context, std::string_view shader_name, shader_stages stage, std::string_view shader_type)
	{
		std::string shader_stage{};
		vk::ShaderStageFlagBits flag{};

		switch (stage)
		{
		case shader_stages::frag:
			shader_stage = "frag";
			flag = vk::ShaderStageFlagBits::eFragment;
			break;
		case shader_stages::vert:
			shader_stage = "vert";
			flag = vk::ShaderStageFlagBits::eVertex;
			break;
		default:
			LOG_ERROR("Unknown shader stage used");
			return {};
		}

		const auto* source_dir = "../assets/shaders/";
		auto shader_filename = std::format("{}{}.{}.{}", source_dir, shader_name, shader_stage, shader_type);

		auto file = egkr::filesystem::open(shader_filename, egkr::file_mode::read, true);

		if (!file.is_valid)
		{
			LOG_ERROR("Failed to open file: {}", shader_filename);
			return {};
		}

		const auto code = filesystem::read_all(file);


		vk::ShaderModuleCreateInfo create_info{};
		create_info
			.setCodeSize(code.size())
			.setPCode((const uint32_t*)(code.data()));

		const vk::ShaderModule shader_module = context->device.logical_device.createShaderModule(create_info, context->allocator);



		vk::PipelineShaderStageCreateInfo stage_create_info{};
		stage_create_info
			.setModule(shader_module)
			.setPName("main")
			.setStage(flag);

		LOG_INFO("Succesfully created {} shader", shader_stage);

		return {create_info, shader_module, stage_create_info};
	}

	shader::shared_ptr shader::create(const vulkan_context* context, pipeline_properties& pipeline_properties)
	{
		return std::make_shared<shader>(context, pipeline_properties);
	}

	shader::shader(const vulkan_context* context, pipeline_properties& pipeline_properties)
		: context_{ context }
	{
		auto frag_shader_module = create_shader_module(context, "Builtin.MaterialShader"sv, shader_stages::frag, "spv"sv);
		auto vert_shader_module = create_shader_module(context, "Builtin.MaterialShader"sv, shader_stages::vert, "spv"sv);

		stages_[shader_stages::vert] = vert_shader_module;
		stages_[shader_stages::frag] = frag_shader_module;

		// Vertex descriptors
		const auto descriptor_type{ vk::DescriptorType::eUniformBuffer };

		vk::DescriptorSetLayoutBinding global_ubo_layout_binding{};
		global_ubo_layout_binding
			.setBinding(0)
			.setDescriptorType(descriptor_type)
			.setImmutableSamplers(nullptr)
			.setStageFlags(vk::ShaderStageFlagBits::eVertex)
			.setDescriptorCount(1);

		vk::DescriptorSetLayoutCreateInfo create_info{};
		create_info
			.setBindings(global_ubo_layout_binding);

		global_descriptor_set_layout_ = context_->device.logical_device.createDescriptorSetLayout(create_info, context_->allocator);

		vk::DescriptorPoolSize pool_size{};
		pool_size
			.setType(descriptor_type)
			.setDescriptorCount(context_->swapchain->get_image_count());

		vk::DescriptorPoolCreateInfo descriptor_pool_create_info{};
		descriptor_pool_create_info
			.setPoolSizes(pool_size)
			.setMaxSets(context_->swapchain->get_image_count());

		global_descriptor_pool_ = context_->device.logical_device.createDescriptorPool(descriptor_pool_create_info, context_->allocator);

		std::array<vk::DescriptorSetLayout, 3> global_layouts = { global_descriptor_set_layout_, global_descriptor_set_layout_, global_descriptor_set_layout_ };

		vk::DescriptorSetAllocateInfo alloc_info{};
		alloc_info
			.setDescriptorPool(global_descriptor_pool_)
			.setSetLayouts(global_layouts);

		global_descriptor_set_ = context_->device.logical_device.allocateDescriptorSets(alloc_info);


		sampler_uses_[0] = texture_use::map_diffuse;

		//Fragment shader descriptors
		std::array<vk::DescriptorType, material_shader_descriptor_count> object_descriptor_types{vk::DescriptorType::eUniformBuffer, vk::DescriptorType::eCombinedImageSampler};
		std::array<vk::DescriptorSetLayoutBinding, material_shader_descriptor_count> object_descriptor_set_layout{};
		std::array<vk::DescriptorPoolSize, material_shader_descriptor_count> object_descriptor_pool{};

		for (auto i{ 0U }; i < object_descriptor_set_layout.size(); ++i)
		{
			object_descriptor_set_layout[i]
				.setBinding(i)
				.setDescriptorCount(1)
				.setDescriptorType(object_descriptor_types[i])
				.setStageFlags(vk::ShaderStageFlagBits::eFragment);

			object_descriptor_pool[i]
				.setType(object_descriptor_types[i])
				.setDescriptorCount(material_shader_sampler_count * max_material_count);
		}

		vk::DescriptorSetLayoutCreateInfo object_create_info{};
		object_create_info
			.setBindings(object_descriptor_set_layout);

		object_descriptor_set_layout_ = context_->device.logical_device.createDescriptorSetLayout(object_create_info, context_->allocator);

		vk::DescriptorPoolCreateInfo object_pool_create_info{};
		object_pool_create_info
			.setPoolSizes(object_descriptor_pool)
			.setMaxSets(max_material_count)
			.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

		object_descriptor_pool_ = context_->device.logical_device.createDescriptorPool(object_pool_create_info, context_->allocator);

		std::array<vk::DescriptorSetLayout, 3> object_layouts = { object_descriptor_set_layout_, object_descriptor_set_layout_, object_descriptor_set_layout_ };

		vk::DescriptorSetAllocateInfo object_alloc_info{};
		object_alloc_info
			.setDescriptorPool(object_descriptor_pool_)
			.setSetLayouts(object_layouts);

		object_descriptor_set_ = context_->device.logical_device.allocateDescriptorSets(object_alloc_info);

		pipeline_properties.descriptor_set_layout = { global_descriptor_set_layout_, object_descriptor_set_layout_ };
		pipeline_properties.shader_stage_info = get_shader_stages();
		pipeline_ = pipeline::create(context_, pipeline_properties);

		auto usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer;
		auto memory_properties = vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
		global_uniform_buffer_ = buffer::create(context_, sizeof(global_uniform_buffer) * 3, usage, memory_properties, true);
		object_uniform_buffer_ = buffer::create(context_, sizeof(material_uniform_object) * 3, usage, memory_properties, true);
	}

	shader::~shader()
	{
		destroy();
	}

	void shader::use()
	{
		const auto& image_index = context_->image_index;
		context_->graphics_command_buffers[image_index].get_handle().bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_->get_handle());
	}

	void shader::update_global_state(const global_uniform_buffer& ubo)
	{
		const auto& command_buffer = context_->graphics_command_buffers[context_->image_index];

		command_buffer.get_handle().bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_->get_layout(), 0, global_descriptor_set_[context_->image_index], nullptr);

		auto range = sizeof(global_uniform_buffer); // one per frame in flight
		auto offset = sizeof(global_uniform_buffer)* context_->image_index;

		global_uniform_buffer_->load_data(offset, range, 0, &ubo);

		vk::DescriptorBufferInfo buffer_info{};
		buffer_info
			.setBuffer(global_uniform_buffer_->get_handle())
			.setOffset(offset)
			.setRange(range);

		vk::WriteDescriptorSet write_set{};
		write_set
			.setDstSet(global_descriptor_set_[context_->image_index])
			.setDstBinding(0)
			.setDstArrayElement(0)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1)
			.setBufferInfo(buffer_info);

		context_->device.logical_device.updateDescriptorSets(write_set, nullptr);

	}

	void shader::update(const geometry_render_data& data)
	{
		auto& command_buffer = context_->graphics_command_buffers[context_->image_index];
		command_buffer.get_handle().pushConstants(pipeline_->get_layout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(float4x4), &data.model);

		auto& object = material_shader_instance_states_[data.material->get_internal_id()];
		auto& object_set = object.descriptor_sets[context_->image_index];

		std::array<vk::WriteDescriptorSet, material_shader_descriptor_count> write_set{};
		auto range = sizeof(material_uniform_object);
		auto offset = sizeof(material_uniform_object) * data.material->get_internal_id();

		//static float accumulate = 0.F;
		//accumulate += data.delta_time;
		//auto colour = (std::sinf(accumulate) + 1) / 2.F;

		material_uniform_object obo{};
		obo.diffuse_colour = data.material->get_diffuse_colour();

		object_uniform_buffer_->load_data(offset, range, 0, &obo);

		uint32_t descriptor_index{};
		uint32_t descriptor_count{};

		auto& generation = object.descriptor_states[descriptor_index].generation[context_->image_index];
		if (generation == invalid_id || generation != data.material->get_generation())
		{
			vk::DescriptorBufferInfo buffer_info{};
			buffer_info
				.setBuffer(object_uniform_buffer_->get_handle())
				.setOffset(offset)
				.setRange(range);

			vk::WriteDescriptorSet write{};
			write
				.setDstSet(object.descriptor_sets[context_->image_index])
				.setDstBinding(descriptor_index)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setDescriptorCount(1)
				.setBufferInfo(buffer_info);

			write_set[descriptor_count] = write;
			++descriptor_count;
			generation = data.material->get_generation();
		}
		++descriptor_index;

		std::array<vk::DescriptorImageInfo, material_shader_sampler_count> image_infos{};

		for (auto sampler_index{ 0U }; sampler_index < image_infos.size(); ++sampler_index)
		{
			auto texture_use = sampler_uses_[sampler_index]; 

			texture::shared_ptr tex{};
			switch (texture_use)
			{
			case texture_use::map_diffuse:
				tex = data.material->get_diffuse_map().texture;
				break;
			default:
				LOG_FATAL("Unknown sampler usage, cannot bind");
					break;
			}

			auto texture = std::dynamic_pointer_cast<vulkan_texture>(tex);
			if (texture->get_generation() == invalid_id)
			{
				texture = std::dynamic_pointer_cast<vulkan_texture>(texture_system::get_default_texture());
			}

			auto& generation = object.descriptor_states[descriptor_index].generation[context_->image_index];
			auto& descriptor_id = object.descriptor_states[descriptor_index].id[context_->image_index];

			if (texture && (texture->get_id() != descriptor_id || texture->get_generation() != generation || generation == invalid_id))
			{
				image_infos[sampler_index]
					.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
					.setImageView(texture->get_view())
					.setSampler(texture->get_sampler());


				vk::WriteDescriptorSet sampler_write{};
				sampler_write
					.setDstSet(object_set)
					.setDstBinding(descriptor_index)
					.setDescriptorCount(1)
					.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
					.setImageInfo(image_infos[sampler_index]);

				write_set[descriptor_count] = sampler_write;
				++descriptor_count;

				if (texture->get_generation() != invalid_id)
				{
					generation = texture->get_generation();
					descriptor_id = texture->get_id();
				}

				++descriptor_index;
			}
		}


		if (descriptor_count > 0)
		{
			context_->device.logical_device.updateDescriptorSets(descriptor_count, write_set.data(), 0, nullptr);
		}

		command_buffer.get_handle().bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_->get_layout(), 1, object_set, nullptr);
	}

	bool shader::acquire_resource(vulkan_material::shared_ptr& material)
	{
		material->set_internal_id(object_uniform_buffer_index_);
		++object_uniform_buffer_index_;

		material_shader_instance_state& object_state = material_shader_instance_states_[material->get_internal_id()];

		std::array<vk::DescriptorSetLayout, 3> object_descriptor_set_layout{object_descriptor_set_layout_, object_descriptor_set_layout_, object_descriptor_set_layout_};
		vk::DescriptorSetAllocateInfo allocate_info{};
		allocate_info
			.setDescriptorPool(object_descriptor_pool_)
			.setSetLayouts(object_descriptor_set_layout);

		object_state.descriptor_sets = context_->device.logical_device.allocateDescriptorSets(allocate_info);

		return true;
	}

	void shader::release_resource(uint32_t object_id)
	{

		context_->device.logical_device.freeDescriptorSets(object_descriptor_pool_, material_shader_instance_states_[object_id].descriptor_sets);
	}

	egkr::vector<vk::PipelineShaderStageCreateInfo> shader::get_shader_stages() const
	{
		egkr::vector<vk::PipelineShaderStageCreateInfo> stages{};

		for (const auto& stage : stages_ | std::views::values)
		{
			stages.emplace_back(stage.stage_create_info);
		}

		return stages;
	}

	void shader::destroy()
	{
		if (object_uniform_buffer_)
		{
			object_uniform_buffer_->destroy();
		}

		if (object_descriptor_pool_)
		{
			context_->device.logical_device.destroyDescriptorPool(object_descriptor_pool_, context_->allocator);
			object_descriptor_pool_ = VK_NULL_HANDLE;
		}

		if (object_descriptor_set_layout_)
		{
			context_->device.logical_device.destroyDescriptorSetLayout(object_descriptor_set_layout_, context_->allocator);
			object_descriptor_set_layout_ = VK_NULL_HANDLE;
		}

		if (global_uniform_buffer_)
		{
			global_uniform_buffer_->destroy();
		}

		if (pipeline_)
		{
			pipeline_->destroy();
		}

		if (global_descriptor_pool_)
		{
			context_->device.logical_device.destroyDescriptorPool(global_descriptor_pool_, context_->allocator);
			global_descriptor_pool_ = VK_NULL_HANDLE;
		}

		if (global_descriptor_set_layout_)
		{
			context_->device.logical_device.destroyDescriptorSetLayout(global_descriptor_set_layout_, context_->allocator);
			global_descriptor_set_layout_ = VK_NULL_HANDLE;
		}

		for (auto& stage : stages_)
		{
			context_->device.logical_device.destroyShaderModule(stage.second.handle, context_->allocator);
		}
		stages_.clear();

	}
}