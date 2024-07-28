#pragma once
#include "pch.h"

#include "renderer/renderpass.h"
#include "texture.h"

#include <unordered_map>

namespace egkr
{
	class shader : public resource
	{
	public:
		enum class cull_mode
		{
			none,
			front,
			back,
			both
		};

		enum class stages
		{
			vertex = 1,
			geometry = 2,
			fragment = 4,
			compute = 8
		};

		enum class attribute_type
		{
			float32_1, float32_2, float32_3, float32_4,
			int8, uint8,
			int16, uint16,
			int32, uint32,
			mat4x4
		};

		enum class uniform_type
		{
			float32_1, float32_2, float32_3, float32_4,
			int8, uint8,
			int16, uint16,
			int32, uint32,
			mat4x4,
			sampler,
			custom
		};

		enum class scope
		{
			global,
			instance,
			local
		};

		struct attribute_configuration
		{
			std::string name{};
			uint16_t size{};
			attribute_type type{};
		};

		struct uniform_configuration
		{
			std::string name{};
			uint16_t size{};
			uint32_t location{};
			uniform_type type{};
			scope scope{};
		};

		enum primitive_topology_type
		{
			none = 0,
			triangle_list = 0x01,
			triangle_strip = 0x02,
			triangle_fan = 0x04,
			line_list = 0x08,
			line_strip = 0x10,
			point_list = 0x20,
			max = point_list << 1
		};

		static inline primitive_topology_type to_primitive_topology(std::string_view topology)
		{
			if (topology.compare("triangle_list") == 0)
			{
				return triangle_list;
			}
			else if (topology.compare("triangle_strip") == 0)
			{
				return triangle_strip;
			}
			else if (topology.compare("triangle_fan") == 0)
			{
				return triangle_fan;
			}
			else if (topology.compare("line_list") == 0)
			{
				return line_list;
			}
			else if (topology.compare("line_strip") == 0)
			{
				return line_strip;
			}
			else if (topology.compare("point_list") == 0)
			{
				return point_list;
			}
			else
			{
				LOG_ERROR("Unrecognized primitive topology type, {}", topology.data());
				return none;
			}
		}

		enum class flags
		{
			depth_write = 0x01,
			depth_test = 0x02,
		};

		struct properties
		{
			std::string name{};
			egkr::vector<attribute_configuration> attributes{};
			egkr::vector<uniform_configuration> uniforms{};
			egkr::vector<stages> stages{};
			egkr::vector<std::string> stage_names{};
			egkr::vector<std::string> stage_filenames{};
			primitive_topology_type topology_types{ primitive_topology_type::triangle_list };
			cull_mode cull_mode{ cull_mode::back };

			uint8_t global_uniform_count{};
			uint8_t global_uniform_sampler_count{};
			uint8_t instance_uniform_count{};
			uint8_t instance_uniform_sampler_count{};
			uint8_t local_uniform_count{};
			flags flags{};
		};

		enum state
		{
			not_created,
			uninitialised,
			initialised
		};

		struct uniform
		{
			uint64_t offset{};
			uint16_t location{};
			uint16_t index{};
			uint16_t size{};
			uint8_t set_index{};
			scope scope{};
			uniform_type type{};
		};

		struct attribute
		{
			std::string name{};
			uint32_t size{};
			attribute_type type{};
		};

		using shared_ptr = std::shared_ptr<shader>;
		static shared_ptr create(const properties& properties, renderpass::renderpass* pass);

		explicit shader(const properties& properties);
		virtual ~shader();

		uint32_t get_uniform_index(std::string_view uniform_name);
		const uniform& get_uniform(uint32_t index);

		const auto& get_bound_scope() const { return bound_scope_; }
		void set_bound_scope(scope scope);

		const auto& get_bound_instance_id() const { return bound_instance_id_; }
		void set_bound_instance_id(uint32_t instance_id);

		auto has_instances() const { return properties_.instance_uniform_count > 0 || properties_.instance_uniform_sampler_count > 0; }

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

		[[nodiscard]] uint32_t get_frame_number() const { return frame_number_; }
		void set_frame_number(uint32_t frame) { frame_number_ = frame; }

		virtual bool use() = 0;
		virtual bool populate(renderpass::renderpass* renderpass, const egkr::vector<std::string>& stage_filenames, const egkr::vector<stages>& shader_stages) = 0;
		virtual void free() = 0;
		virtual bool bind_instances(uint32_t instance_id) = 0;
		virtual bool apply_instances(bool needs_update) = 0;
		virtual bool bind_globals() = 0;
		virtual bool apply_globals(bool needs_update) = 0;
		virtual uint32_t acquire_instance_resources(const egkr::vector<texture_map::shared_ptr>& texture_maps) = 0;
		virtual bool set_uniform(const uniform& uniform, const void* value) = 0;
	private:
		bool add_attribute(const attribute_configuration& configuration);
		bool add_sampler(const uniform_configuration& configuration);
		bool add_uniform(const uniform_configuration& configuration);

		void add_uniform(std::string_view uniform_name, uint32_t size, uniform_type type, scope scope, uint32_t set_location, bool is_sampler);
		bool is_uniform_name_valid(std::string_view uniform_name);
		bool is_uniform_add_state_valid();

	protected:
		properties properties_{};
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
		scope bound_scope_{};
		uint32_t bound_instance_id_{};
		uint64_t bound_ubo_offset_{};

		std::unordered_map<std::string, uint32_t> uniform_id_by_name_{};
		egkr::vector<uniform> uniforms_{};

		egkr::vector<attribute> attributes_{};
		state state_{ state::not_created };

		egkr::vector<range> push_const_ranges_;
		uint16_t attribute_stride_{};

		primitive_topology_type topology_types_{};
		uint16_t bound_pipeline_index_{};

		flags flags_{};
		uint32_t frame_number_{};
	};
	ENUM_CLASS_OPERATORS(shader::primitive_topology_type)
	ENUM_CLASS_OPERATORS(shader::flags)
}

