#include "material_system.h"

#include "engine/engine.h"
#include "texture_system.h"
#include "systems/resource_system.h"
#include "systems/shader_system.h"
#include "systems/light_system.h"

namespace egkr
{

    static material_system::unique_ptr material_system_{};

    material_system* material_system::create()
    {
	material_system_ = std::make_unique<material_system>();
	return material_system_.get();
    }

    material_system::material_system(): max_material_count_{1024} { }

    material_system::~material_system() { shutdown(); }

    bool material_system::init()
    {
	if (max_material_count_ == 0)
	{
	    LOG_FATAL("Material max count must be > 0");
	    return false;
	}

	registered_materials_.reserve(max_material_count_);
	registered_materials_by_name_.reserve(max_material_count_);

	if (!create_default_material())
	{
	    LOG_FATAL("Failed to create default material");
	    return false;
	}


	return true;
    }

    bool material_system::shutdown()
    {
	if (material_system_->default_material_)
	{
	    engine::get()->get_renderer()->free_material(material_system_->default_material_.get());
	    material_system_->default_material_.reset();
	}

	for (auto& material : material_system_->registered_materials_)
	{
	    engine::get()->get_renderer()->free_material(material.get());
	}
	material_system_->registered_materials_.clear();

	material_system_->registered_materials_by_name_.clear();
	return true;
    }

    material::shared_ptr material_system::get_default_material() { return material_system_->default_material_; }

    material::shared_ptr material_system::acquire(const std::string& name)
    {
	auto material_resource = resource_system::load(name, resource::type::material, nullptr);

	auto* material_config = (material::properties*)material_resource->data;

	auto material = acquire(*material_config);

	resource_system::unload(material_resource);

	return material;
    }

    material::shared_ptr material_system::acquire(const material::properties& properties)
    {
	if (strcmp(properties.name.data(), default_material_name_.data()) == 0)
	{
	    return material_system_->default_material_;
	}


	if (material_system_->registered_materials_by_name_.contains(properties.name))
	{
	    auto material_handle = material_system_->registered_materials_by_name_[properties.name.data()];
	    return material_system_->registered_materials_[material_handle];
	}

	uint32_t material_id = (uint32_t)material_system_->registered_materials_.size();

	if (material_id >= material_system_->max_material_count_)
	{
	    LOG_FATAL("Exceeded max texture count");
	    return nullptr;
	}

	auto new_material = material::create(properties);
	load_material(properties, new_material);

	//if (new_material->get_generation() == invalid_id)
	//{
	//	new_material->set_generation(0);
	//}
	//else
	//{
	//	new_material->increment_generation();
	//}

	auto shader = shader_system::get_shader(new_material->get_shader_id());
	if (material_system_->material_shader_id_ == invalid_32_id && properties.shader_name == "Shader.Material")
	{
	    material_system_->material_shader_id_ = shader->get_id();
	    material_shader_uniform_location locations{};
	    locations.projection = shader->get_uniform_index("projection");
	    locations.view = shader->get_uniform_index("view");
	    locations.shininess = shader->get_uniform_index("shininess");
	    locations.ambient_colour = shader->get_uniform_index("ambient_colour");
	    locations.diffuse_colour = shader->get_uniform_index("diffuse_colour");
	    locations.diffuse_texture = shader->get_uniform_index("diffuse_texture");
	    locations.specular_texture = shader->get_uniform_index("specular_texture");
	    locations.normal_texture = shader->get_uniform_index("normal_texture");
	    locations.view_position = shader->get_uniform_index("view_position");
	    locations.model = shader->get_uniform_index("model");
	    locations.mode = shader->get_uniform_index("mode");
	    locations.point_light = shader->get_uniform_index("point_lights");
	    locations.directional_light = shader->get_uniform_index("dir_light");
	    locations.num_point_lights = shader->get_uniform_index("num_point_lights");
	    material_system_->material_locations_ = locations;
	}
	else if (material_system_->ui_shader_id_ == invalid_32_id && properties.shader_name == "Shader.UI")
	{
	    material_system_->ui_shader_id_ = shader->get_id();
	    ui_shader_uniform_location locations{};
	    locations.projection = shader->get_uniform_index("projection");
	    locations.view = shader->get_uniform_index("view");
	    locations.diffuse_colour = shader->get_uniform_index("diffuse_colour");
	    locations.diffuse_texture = shader->get_uniform_index("diffuse_texture");
	    locations.model = shader->get_uniform_index("model");
	    material_system_->ui_locations_ = locations;
	}

	material_system_->registered_materials_.push_back(new_material);
	material_system_->registered_materials_by_name_[properties.name.data()] = material_id;
	return new_material;
    }

    bool material_system::release(const material::shared_ptr& material)
    {
	//TODO ew
	if (material_system_->registered_materials_by_name_.contains(material->get_name()))
	{
	    //Can't clear out vector as things access it by index
	    material_system_->registered_materials_[material_system_->registered_materials_by_name_[material->get_name()]]->free();
	    material_system_->registered_materials_by_name_.erase(material->get_name());
	    return true;
	}
	return false;
    }

    void material_system::apply_global(
        uint32_t shader_id, const frame_data& frame_data, const float4x4& projection, const float4x4& view, const float4& ambient_colour, const float3& view_position, uint32_t mode)
    {
	auto shader = shader_system::get_shader(shader_id);

	if (shader->get_draw_index() == frame_data.draw_index && shader->get_frame_number() == frame_data.frame_number)
	{
	    return;
	}

	if (shader_id == material_system_->material_shader_id_)
	{
	    shader_system::set_uniform(material_system_->material_locations_.projection, &projection);
	    shader_system::set_uniform(material_system_->material_locations_.view, &view);
	    shader_system::set_uniform(material_system_->material_locations_.ambient_colour, &ambient_colour);
	    shader_system::set_uniform(material_system_->material_locations_.view_position, &view_position);
	    shader_system::set_uniform(material_system_->material_locations_.mode, &mode);
	}
	else
	{
	    shader_system::set_uniform(material_system_->ui_locations_.projection, &projection);
	    shader_system::set_uniform(material_system_->ui_locations_.view, &view);
	}
	shader_system::apply_global(true);
	shader->set_frame_number(frame_data.frame_number);
    }

    void material_system::apply_instance(const material::shared_ptr& material, bool needs_update)
    {
	shader_system::bind_instance(material->get_internal_id());

	if (needs_update)
	{
	    if (material->get_shader_id() == material_system_->material_shader_id_)
	    {
		shader_system::set_uniform(material_system_->material_locations_.diffuse_colour, &material->get_diffuse_colour());
		shader_system::set_uniform(material_system_->material_locations_.diffuse_texture, &material->get_diffuse_map());
		shader_system::set_uniform(material_system_->material_locations_.specular_texture, &material->get_specular_map());
		shader_system::set_uniform(material_system_->material_locations_.normal_texture, &material->get_normal_map());
		shader_system::set_uniform(material_system_->material_locations_.shininess, &material->get_shininess());
		shader_system::set_uniform(material_system_->material_locations_.directional_light, light_system::get_directional_light());
		shader_system::set_uniform(material_system_->material_locations_.point_light, light_system::get_point_lights().data());
		auto num_point_lights = light_system::point_light_count();
		shader_system::set_uniform(material_system_->material_locations_.num_point_lights, &num_point_lights);
	    }
	    else if (material->get_shader_id() == material_system_->ui_shader_id_)
	    {
		shader_system::set_uniform(material_system_->ui_locations_.diffuse_colour, &material->get_diffuse_colour());
		shader_system::set_uniform(material_system_->ui_locations_.diffuse_texture, &material->get_diffuse_map());
	    }
	}
	shader_system::apply_instance(needs_update);
    }

    void material_system::apply_local(const material::shared_ptr& material, const float4x4& model)
    {
	auto shader_id = material->get_shader_id();
	if (shader_id == material_system_->material_shader_id_)
	{
	    return shader_system::set_uniform(material_system_->material_locations_.model, &model);
	}
	else if (shader_id == material_system_->ui_shader_id_)
	{
	    shader_system::set_uniform(material_system_->ui_locations_.model, &model);
	}
    }

    bool material_system::create_default_material()
    {
	material::properties properties{};
	properties.name = "default";
	properties.diffuse_map_name = default_diffuse_name;
	properties.specular_map_name = default_specular_name;
	properties.normal_map_name = default_normal_name;
	properties.diffuse_colour = float4{1.F};
	properties.shader_name = "Shader.Material";
	material_system_->default_material_ = material::create(properties);
	egkr::vector<texture_map::texture_map::shared_ptr> texture_maps{
	    material_system_->default_material_->get_diffuse_map(), material_system_->default_material_->get_specular_map(), material_system_->default_material_->get_normal_map()};

	for (auto map : texture_maps)
	{
	    map->acquire();
	}

	auto mat_shader = shader_system::get_shader("Shader.Material");
	auto id = mat_shader->acquire_instance_resources(texture_maps);
	material_system_->default_material_->set_internal_id(id);
	return true;
    }

    bool material_system::load_material(const material::properties& properties, material::shared_ptr& material)
    {
	texture_map::properties diffuse_properties{.minify = texture_map::filter::linear,
	    .magnify = texture_map::filter::linear,
	    .repeat_u = texture_map::repeat::repeat,
	    .repeat_v = texture_map::repeat::repeat,
	    .repeat_w = texture_map::repeat::repeat,
	    .map_use = texture_map::use::map_diffuse};

	auto diffuse_map = texture_map::texture_map::create(diffuse_properties);

	//Get diffuse map
	diffuse_map->map_texture = texture_system::acquire(properties.diffuse_map_name);
	if (diffuse_map->map_texture == nullptr)
	{
	    LOG_WARN("Failed to find texture: {} for material {}. Setting default", properties.diffuse_map_name.data(), properties.name.data());
	    diffuse_map->map_texture = texture_system::get_default_texture();
	}
	diffuse_map->acquire();

	texture_map::properties specular_properties = diffuse_properties;
	specular_properties.map_use = texture_map::use::map_specular;

	auto specular_map = texture_map::texture_map::create(specular_properties);

	specular_map->map_texture = texture_system::acquire(properties.specular_map_name);
	if (specular_map->map_texture == nullptr)
	{
	    LOG_WARN("Failed to find texture: {} for material {}. Setting default", properties.specular_map_name.data(), properties.name.data());
	    specular_map->map_texture = texture_system::get_default_specular_texture();
	}
	specular_map->acquire();

	texture_map::properties normal_properties = diffuse_properties;
	normal_properties.map_use = texture_map::use::map_normal;

	auto normal_map = texture_map::texture_map::create(normal_properties);
	normal_map->map_texture = texture_system::acquire(properties.normal_map_name);
	if (normal_map->map_texture == nullptr)
	{
	    LOG_WARN("Failed to find texture: {} for material {}. Setting default", properties.normal_map_name.data(), properties.name.data());
	    normal_map->map_texture = texture_system::get_default_normal_texture();
	}
	normal_map->acquire();

	egkr::vector<texture_map::texture_map::shared_ptr> maps{diffuse_map, specular_map, normal_map};
	auto shader = shader_system::get_shader(properties.shader_name);
	auto id = shader->acquire_instance_resources(maps);

	//material.reset();
	material = material::create(properties);
	if (material->get_generation() == invalid_32_id)
	{
	    material->set_generation(0);
	}
	else
	{
	    material->increment_generation();
	}
	material->set_diffuse_colour(properties.diffuse_colour);
	material->set_shininess(properties.shininess);
	material->set_internal_id(id);
	//material->set_name(properties.name);
	material->set_diffuse_map(diffuse_map);
	material->set_specular_map(specular_map);
	material->set_normal_map(normal_map);
	return true;
    }
}
