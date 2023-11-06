#pragma once
#include "pch.h"
#include "pipeline.h"
#include "renderer/renderer_types.h"
#include "buffer.h"

#include "resources/shader.h"

#include <vulkan/vulkan.hpp>

namespace egkr::shader
{

	constexpr static uint32_t max_material_count{1024};
	constexpr static uint32_t material_shader_descriptor_count{ 2 };
	constexpr static uint32_t material_shader_sampler_count{ 1 };

	constexpr static uint8_t BINDING_INDEX_UBO{ 0 };
	constexpr static uint8_t BINDING_INDEX_SAMPLER{ 1 };

	constexpr static uint8_t DESCRIPTOR_SET_INDEX_GLOBAL{ 0 };
	constexpr static uint8_t DESCRIPTOR_SET_INDEX_INSTANCE{ 1 };

	struct vulkan_descriptor_set_configuration
	{
		egkr::vector<vk::DescriptorSetLayoutBinding> bindings{};
	};

	struct vulkan_stage_configuration
	{
		vk::ShaderStageFlagBits stage{};
		std::string filename{};
	};

	struct vulkan_configuration
	{
		uint8_t stage_count{};
		egkr::vector<vulkan_stage_configuration> stages{};
		egkr::vector<vk::DescriptorPoolSize> pool_sizes{};

		uint16_t max_descriptor_set_count{};
		//0 = global, 1 = instance
		std::array<vulkan_descriptor_set_configuration, 2> descriptor_sets{};
		egkr::vector<vk::VertexInputAttributeDescription> attributes{};
	};
	
	struct vulkan_descriptor_state
	{
		std::array<uint8_t, 3> ids{ invalid_8_id, invalid_8_id, invalid_8_id };
		std::array<uint8_t, 3> generations{ invalid_8_id, invalid_8_id, invalid_8_id };

	};

	struct vulkan_stage
	{
		vk::ShaderModuleCreateInfo create_info{};
		vk::ShaderModule handle{};
		vk::PipelineShaderStageCreateInfo shader_stage_create_info{};
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

		egkr::vector<texture::texture_map*> instance_textures{};
	};

	struct vulkan_state
	{
		const vulkan_context* context_{};

		uint32_t id{invalid_32_id};
		vulkan_configuration configuration{};
		renderpass::vulkan_renderpass* renderpass{};
		egkr::vector<vulkan_stage> stages;

		vk::DescriptorPool descriptor_pool{};

		//0 = global, 1 = instance
		egkr::vector<vk::DescriptorSetLayout> descriptor_set_layout{2};

		// one poer frame
		egkr::vector<vk::DescriptorSet> global_descriptor_sets{ 3 };

		buffer::shared_ptr uniform_buffer{};

		pipeline::shared_ptr pipeline{};

		egkr::vector<vulkan_instance_state> instance_states{};

		void* mapped_uniform_buffer_memory{};
	};

	static std::unordered_map<attribute_type, vk::Format> vulkan_attribute_types
	{
		{attribute_type::float32_1, vk::Format::eR32Sfloat},
		{attribute_type::float32_2, vk::Format::eR32G32Sfloat},
		{attribute_type::float32_3, vk::Format::eR32G32B32Sfloat},
		{attribute_type::float32_4, vk::Format::eR32G32B32A32Sfloat},
		{attribute_type::int8, vk::Format::eR8Sint},
		{attribute_type::uint8, vk::Format::eR8Uint},
		{attribute_type::int16, vk::Format::eR16Sint},
		{attribute_type::uint16, vk::Format::eR16Uint},
		{attribute_type::int32, vk::Format::eR32Sint},
		{attribute_type::uint32, vk::Format::eR32Uint},
	};

}
