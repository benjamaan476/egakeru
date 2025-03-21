#pragma once
#include "pch.h"
#include "pipeline.h"
#include "renderer/renderer_types.h"
#include "renderer/renderbuffer.h"
#include "vulkan_renderbuffer.h"

#include "resources/shader.h"

#include <vulkan/vulkan.hpp>

namespace egkr
{
	constexpr static uint32_t max_material_count{1024};
	constexpr static uint32_t material_shader_descriptor_count{ 2 };
	constexpr static uint32_t material_shader_sampler_count{ 1 };

	constexpr static uint8_t DESCRIPTOR_SET_INDEX_GLOBAL{ 0 };
	constexpr static uint8_t DESCRIPTOR_SET_INDEX_INSTANCE{ 1 };

	struct vulkan_descriptor_set_configuration
	{
		egkr::vector<vk::DescriptorSetLayoutBinding> bindings;
		uint32_t binding_count{};
		uint8_t sampler_binding_index{};
	};

	struct vulkan_stage_configuration
	{
		vk::ShaderStageFlagBits stage{};
		std::string filename;
	};

	struct vulkan_configuration
	{
		uint8_t stage_count{};
		egkr::vector<vulkan_stage_configuration> stages;
		egkr::vector<vk::DescriptorPoolSize> pool_sizes;

		uint16_t max_descriptor_set_count{};
		//0 = global, 1 = instance
		std::array<vulkan_descriptor_set_configuration, 2> descriptor_sets{};
		egkr::vector<vk::VertexInputAttributeDescription> attributes;
	};
	
	struct vulkan_descriptor_state
	{
		std::array<uint8_t, 3> ids{ invalid_8_id, invalid_8_id, invalid_8_id };
		std::array<uint8_t, 3> generations{ invalid_8_id, invalid_8_id, invalid_8_id };

	};

	struct vulkan_stage
	{
		vk::ShaderModuleCreateInfo create_info;
		vk::ShaderModule handle;
		vk::PipelineShaderStageCreateInfo shader_stage_create_info;
	};

	struct vulkan_descriptor_set_state
	{
		egkr::vector<vk::DescriptorSet> descriptor_sets{ 3 };
		egkr::vector<vulkan_descriptor_state> descriptor_states{3};
	};

	struct vulkan_instance_state
	{
		uint32_t id{};
		uint64_t offset{};
		vulkan_descriptor_set_state descriptor_set_state{};

		egkr::vector<texture_map::shared_ptr> instance_textures;
	};

	static std::unordered_map<shader::attribute_type, vk::Format> vulkan_attribute_types
	{
		{ shader::attribute_type::float32_1, vk::Format::eR32Sfloat},
		{ shader::attribute_type::float32_2, vk::Format::eR32G32Sfloat},
		{ shader::attribute_type::float32_3, vk::Format::eR32G32B32Sfloat},
		{ shader::attribute_type::float32_4, vk::Format::eR32G32B32A32Sfloat},
		{ shader::attribute_type::int8, vk::Format::eR8Sint},
		{ shader::attribute_type::uint8, vk::Format::eR8Uint},
		{ shader::attribute_type::int16, vk::Format::eR16Sint},
		{ shader::attribute_type::uint16, vk::Format::eR16Uint},
		{ shader::attribute_type::int32, vk::Format::eR32Sint},
		{ shader::attribute_type::uint32, vk::Format::eR32Uint},
	};

	enum class topology_class
	{
		point = 0,
		line = 1,
		triangle = 2,
		max = triangle + 1
	};

	class vulkan_shader : public shader::shader
	{
	public:
		using shared_ptr = std::shared_ptr<shader>;
		static shared_ptr create(const vulkan_context* context, const properties& properties);

		vulkan_shader(const vulkan_context* context, const properties& properties);
		~vulkan_shader() override;

		void free() override;
		bool use() override;
		bool populate(renderpass::renderpass* pass, const egkr::vector<std::string>& stage_filenames, const egkr::vector<stages>& shader_stages) override;
		bool bind_instances(uint32_t instance_id) override;
		bool apply_instances(bool needs_update) override;
		bool bind_globals() override;

		bool apply_globals(bool needs_update) override;
		uint32_t acquire_instance_resources(const egkr::vector<texture_map::shared_ptr>& texture_maps) override;
		bool set_uniform(const uniform& uniform, const void* value) override;

	private:
		vulkan_stage create_module(const vulkan_stage_configuration& configuration);
	private:
		const vulkan_context* context_{};
		vulkan_configuration configuration{};
		renderpass::vulkan_renderpass* renderpass{};
		egkr::vector<vulkan_stage> vulkan_stages;

		vk::DescriptorPool descriptor_pool;

		//0 = global, 1 = instance
		egkr::vector<vk::DescriptorSetLayout> descriptor_set_layout{2};

		// one poer frame
		egkr::vector<vk::DescriptorSet> global_descriptor_sets{ 3 };

		renderbuffer::renderbuffer::shared_ptr uniform_buffer;

		// One for each class; point, line, tri
		egkr::vector<pipeline::shared_ptr> pipelines_{3};
		vk::PrimitiveTopology current_topology_{};

		egkr::vector<vulkan_instance_state> instance_states;

		void* mapped_uniform_buffer_memory{};
	};

}
