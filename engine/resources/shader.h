#pragma once
#include "pch.h"

#include "resource.h"
#include "texture.h"

#include <unordered_map>

namespace egkr
{
	struct range
	{
		uint32_t offset{};
		uint32_t size{};
	};
	enum class shader_stage
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
		egkr::vector<shader_stage> stages{};
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
		static shared_ptr create(const void* renderer_context, const shader_properties& properties);

		shader(const void* renderer_context, const shader_properties& properties);

		uint32_t get_uniform_index(std::string_view uniform_name);
		const shader_uniform& get_uniform(uint32_t index);
		void set_uniform(const shader_uniform& uniform, const void* value);
		void use();

		void bind_globals();
		void bind_instance();

		const auto get_bound_scope() const {return bound_scope_; }
		void set_bound_scope(shader_scope scope);

		void set_bound_instance_id(uint32_t instance_id);
	private:
		bool add_attribute(const attribute_configuration& configuration);
		bool add_sampler(const uniform_configuration& configuration);
		bool add_uniform(const uniform_configuration& configuration);
		uint32_t get_shader_id(std::string_view name);


		void add_uniform(std::string_view uniform_name, uint32_t size, shader_uniform_type type, shader_scope scope, uint32_t set_location, bool is_sampler);
		bool is_uniform_name_valid(std::string_view uniform_name);
		bool is_uniform_add_state_valid();

	private:
		std::string name_{};
		bool use_instances_{};
		bool use_locals_{};
		uint64_t requried_ubo_alignment_{};

		uint64_t globol_ubo_size_{};
		uint64_t global_ubo_stride_{};
		uint64_t global_ubo_offset_{};

		uint64_t ubo_size_{};
		uint64_t ubo_stride_{};

		uint64_t push_constant_size_{};
		uint64_t push_constan_stride_{};
		egkr::vector<texture::shared_ptr> global_textures_{};

		uint8_t instance_texture_count_{};
		shader_scope bound_scope_{};
		uint32_t bound_instance_id_{};
		uint32_t bound_ubo_offset_{};

		std::unordered_map<std::string, uint32_t> uniform_id_by_name_{};
		egkr::vector<shader_uniform> uniforms_{};

		egkr::vector<shader_attribute> attributes_{};
		shader_state state_{shader_state::not_created};

		uint8_t  push_constant_range_uniform_count_{};
		egkr::vector<range> push_const_ranges_{32};
		uint16_t attribute_stride_{};

		const void* renderer_context_{};
	};
}