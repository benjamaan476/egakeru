#pragma once
#include "pch.h"

#include "texture.h"

#include <unordered_map>

namespace egkr
{
	enum class shader_stages
	{
		vertex = 1,
		geometry = 2,
		fragment = 4,
		compute = 8
	};

	enum class shader_attribute_type
	{
		float32_1, float32_2, float32_3, float32_4,
		int8, uint8,
		int16, uint16,
		int32, uint32,
		mat4x4
	};

	enum class shader_uniform_type
	{
		float32_1, float32_2, float32_3, float32_4,
		int8, uint8,
		int16, uint16,
		int32, uint32,
		mat4x4,
		sampler,
		custom
	};

	enum class shader_scope
	{
		global,
		instance,
		local
	};

	struct attribute_configuration
	{
		std::string name{};
		uint8_t size{};
		shader_attribute_type type{};
	};

	struct uniform_configuration
	{
		std::string name{};
		uint8_t size{};
		uint32_t location{};
		shader_uniform_type type{};
		shader_scope scope{};
	};

	struct shader_properties
	{
		std::string name{};
		bool use_instance{};
		bool use_local{};
		egkr::vector<attribute_configuration> attributes{};
		egkr::vector<uniform_configuration> uniforms{};
		std::string renderpass_name{};
		egkr::vector<shader_stages> stages{};
		egkr::vector<std::string> stage_names{};
		egkr::vector<std::string> stage_filenames{};
	};

	enum shader_state
	{
		not_created,
		uninitialised,
		initialised
	};

	struct shader_uniform
	{
		uint64_t offset{};
		uint16_t location{};
		uint16_t index{};
		uint16_t size{};
		uint8_t set_index{};
		shader_scope scope{};
		shader_uniform_type type{};
	};

	struct shader_attribute
	{
		std::string name{};
		uint32_t size{};
		shader_attribute_type type{};
	};

	class shader : public resource
	{
	public:
		using shared_ptr = std::shared_ptr<shader>;
		static shared_ptr create(const renderer_frontend* renderer_context, const shader_properties& properties);

		shader(const renderer_frontend* renderer_context, const shader_properties& properties);

		uint32_t get_uniform_index(std::string_view uniform_name);
		const shader_uniform& get_uniform(uint32_t index);

		const auto& get_bound_scope() const {return bound_scope_; }
		void set_bound_scope(shader_scope scope);

		const auto& get_bound_instance_id() const { return bound_instance_id_; }
		void set_bound_instance_id(uint32_t instance_id);

		auto has_instances() const { return use_instances_; }

		const auto& get_attributes() const { return attributes_; }
		const auto& get_uniforms() const { return uniforms_; }
		const auto& get_push_constant_ranges() const { return push_const_ranges_; }

		const auto& get_global_ubo_stride() const { return global_ubo_stride_; }
		const auto& get_global_ubo_size() const { return global_ubo_size_; }
		void set_global_ubo_stride(uint64_t stride) { global_ubo_stride_ = stride; }

		const auto& get_ubo_stride() const { return ubo_stride_; }
		const auto& get_ubo_size() const { return ubo_size_; }
		void set_ubo_stride(uint64_t stride) { ubo_stride_ = stride; }

		const auto& get_global_ubo_offset() const { return global_ubo_offset_; }
		void set_bound_ubo_offset(uint64_t offset) { bound_ubo_offset_ = offset; }
		const auto& get_bound_ubo_offset() const { return bound_ubo_offset_; }

		void set_global_texture(uint32_t index, texture_map* map);

		const auto& get_attribute_stride() const { return attribute_stride_; }
	private:
		bool add_attribute(const attribute_configuration& configuration);
		bool add_sampler(const uniform_configuration& configuration);
		bool add_uniform(const uniform_configuration& configuration);

		void add_uniform(std::string_view uniform_name, uint32_t size, shader_uniform_type type, shader_scope scope, uint32_t set_location, bool is_sampler);
		bool is_uniform_name_valid(std::string_view uniform_name);
		bool is_uniform_add_state_valid();

	private:
		bool use_instances_{};
		bool use_locals_{};
		uint64_t requried_ubo_alignment_{};

		uint64_t global_ubo_size_{};
		uint64_t global_ubo_stride_{};
		uint64_t global_ubo_offset_{};

		uint64_t ubo_size_{};
		uint64_t ubo_stride_{};

		uint64_t push_constant_size_{};
		uint64_t push_constan_stride_{};
		egkr::vector<std::shared_ptr<texture_map>> global_textures_{};

		uint8_t instance_texture_count_{};
		shader_scope bound_scope_{};
		uint32_t bound_instance_id_{};
		uint32_t bound_ubo_offset_{};

		std::unordered_map<std::string, uint32_t> uniform_id_by_name_{};
		egkr::vector<shader_uniform> uniforms_{};

		egkr::vector<shader_attribute> attributes_{};
		shader_state state_{shader_state::not_created};

		egkr::vector<range> push_const_ranges_;
		uint16_t attribute_stride_{};

		const renderer_frontend* renderer_context_{};
	};
}