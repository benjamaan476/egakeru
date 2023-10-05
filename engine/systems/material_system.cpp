#include "material_system.h"

#include "texture_system.h"
#include "platform/filesystem.h"

namespace egkr
{
	static material_system::unique_ptr material_system_{};

	bool material_system::create(const void* renderer_context)
	{
		material_system_ = std::make_unique<material_system>(renderer_context);
		return material_system::init();
	}

	material_system::material_system(const void* renderer_context)
		: renderer_context_{ renderer_context }, max_material_count_{ 1024 }
	{
	}

	material_system::~material_system()
	{
		shutdown();
	}

	bool material_system::init()
	{
		if (material_system_->max_material_count_ == 0)
		{
			LOG_FATAL("Material max count must be > 0");
			return false;
		}

		material_system_->registered_materials_.reserve(material_system_->max_material_count_);
		material_system_->registered_materials_by_name_.reserve(material_system_->max_material_count_);

		if (!create_default_material())
		{
			LOG_FATAL("Failed to create default material");
			return false;
		}


		return true;
	}

	void material_system::shutdown()
	{
		material_system_->default_material_.reset();
		material_system_->registered_materials_.clear();
		material_system_->registered_materials_by_name_.clear();
	}

	material::shared_ptr material_system::acquire(std::string_view name)
	{
		constexpr const std::string_view format_string{ "../assets/materials/{}.{}" };

		const auto material_filename = std::format(format_string, name, "emt");

		const auto properties = load_configuration_file(material_filename);

		return acquire(properties);
	}

	material::shared_ptr material_system::acquire(const material_properties& properties)
	{
		if (strcmp(properties.name.data(), default_material_name_.data()) == 0)
		{
			return material_system_->default_material_;
		}


		if (material_system_->registered_materials_by_name_.contains(properties.name.data()))
		{
			auto material_handle = material_system_->registered_materials_by_name_[properties.name.data()];
			return material_system_->registered_materials_[material_handle];
		}

		uint32_t material_id = material_system_->registered_materials_.size();

		if (material_id >= material_system_->max_material_count_)
		{
			LOG_FATAL("Exceeded max texture count");
			return nullptr;
		}

		auto new_material = material::create(material_system_->renderer_context_);
		load_material(properties, new_material);

		//if (new_material->get_generation() == invalid_id)
		//{
		//	new_material->set_generation(0);
		//}
		//else
		//{
		//	new_material->increment_generation();
		//}

		material_system_->registered_materials_.push_back(new_material);
		material_system_->registered_materials_by_name_[properties.name.data()] = material_id;
		return new_material;
	}

	bool material_system::create_default_material()
	{
		material_system_->default_material_ = material::create(material_system_->renderer_context_);
		return true;
	}

	bool material_system::load_material(const material_properties& properties, material::shared_ptr& material)
	{

		//Get diffuse map
		auto texture = texture_system::acquire(properties.diffuse_map_name);
		if (texture == nullptr)
		{
			LOG_WARN("Failed to find texture: {} for material {}. Setting default", properties.diffuse_map_name.data(), properties.name.data());
			texture = texture_system::get_default_texture();
		}


			auto temp_material = material::create(material_system_->renderer_context_);

			material.reset();
			material = std::move(temp_material);
			if (material->get_generation() == invalid_id)
			{
				material->set_generation(0);
			}
			else
			{
				material->increment_generation();
			}
		material->set_diffuse_colour(properties.diffuse_colour);
		material->set_name(properties.name);
		material->set_diffuse_map({ std::move(texture),texture_use::map_diffuse });

		return true;
	}

	material_properties material_system::load_configuration_file(std::string_view path)
	{
		material_properties properties{};

		auto handle = filesystem::open(path, file_mode::read, false);
		if (!handle.is_valid)
		{
			LOG_ERROR("Failed to open material file: {}", path.data());
			return properties;
		}

		auto line = filesystem::read_line(handle, 511); 
		uint32_t line_number{};
		for(; !line.empty(); line = filesystem::read_line(handle, 511), ++line_number)
		{
			std::string line_string{ line.begin(), line.end() };
			trim(line_string);
			
			if (line.empty() || line[0] == '#' || line_string == "")
			{
				//line = filesystem::read_line(handle, 511);
				continue;
			}



			auto split_index = line_string.find_first_of('=');
			if (split_index == std::string::npos)
			{
				LOG_WARN("Potential formatting issue found in file {}: '=' token not found on line number {}.", path.data(), line_number);
				continue;
			}

			auto variable_name = line_string.substr(0, split_index);
			trim(variable_name);

			auto value = line_string.substr(split_index +  1);
			trim(value);

			if (variable_name == "version")
			{
				//TODO version
			}
			else if (variable_name == "name")
			{
				properties.name = value;
			}
			else if (variable_name == "diffuse_map_name")
			{
				properties.diffuse_map_name = value;
			}
			else if (variable_name == "diffuse_colour")
			{
				std::stringstream ss{ value };
				float x, y, z, w;
				ss >> x >> y >> z >> w;

				properties.diffuse_colour = { x, y, z, w };
			}

		}

		return properties;
	}
}