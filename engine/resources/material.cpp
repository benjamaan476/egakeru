#include "material.h"

#include "engine/engine.h"

#include "resources/texture.h"
#include "systems/texture_system.h"
#include "systems/shader_system.h"

namespace egkr
{
    material::shared_ptr material::create(const properties& properties)
    {
	auto mat = std::make_shared<material>(properties);
	return mat;
    }

    material::material(const properties& material_properties)
        : resource(0, 0, material_properties.name), diffuse_colour_{material_properties.diffuse_colour}, material_type{material_properties.material_type}, shader_name_{material_properties.shader_name}
    {
	// diffuse_map_ = texture_map::create({});
	// if (material_properties.diffuse_map_name == default_diffuse_name)
	// {
	//     diffuse_map_->map_texture = texture_system::get_default_diffuse_texture();
	// }
	// else
	// {
	//     diffuse_map_->map_texture = texture_system::acquire(material_properties.diffuse_map_name);
	// }
	//
	// specular_map_ = texture_map::create({});
	// if (material_properties.specular_map_name == default_specular_name)
	// {
	//     specular_map_->map_texture = texture_system::get_default_specular_texture();
	// }
	// else
	// {
	//     specular_map_->map_texture = texture_system::acquire(material_properties.specular_map_name);
	// }
	//
	// normal_map_ = texture_map::create({});
	// if (material_properties.normal_map_name == default_normal_name)
	// {
	//     normal_map_->map_texture = texture_system::get_default_diffuse_texture();
	// }
	// else
	// {
	//     normal_map_->map_texture = texture_system::acquire(material_properties.normal_map_name);
	// }
	egkr::vector<texture_map::texture_map::shared_ptr> maps;
	if (material_properties.texture_maps.contains("diffuse"))
	{
	    diffuse_map_ = texture_map::create(material_properties.texture_maps.at("diffuse").second);
	    diffuse_map_->map_texture = texture_system::acquire(material_properties.texture_maps.at("diffuse").first);
	}
	else
	{
	    diffuse_map_ = texture_map::create({});
	    diffuse_map_->map_texture = texture_system::get_default_diffuse_texture();
	}
	maps.push_back(diffuse_map_);

	if (material_properties.texture_maps.contains("specular"))
	{
	    specular_map_ = texture_map::create(material_properties.texture_maps.at("specular").second);
	    specular_map_->map_texture = texture_system::acquire(material_properties.texture_maps.at("specular").first);
	}
	else
	{
	    specular_map_ = texture_map::create({});
	    specular_map_->map_texture = texture_system::get_default_specular_texture();
	}
	maps.push_back(specular_map_);

	if (material_properties.texture_maps.contains("normal"))
	{
	    normal_map_ = texture_map::create(material_properties.texture_maps.at("normal").second);
	    normal_map_->map_texture = texture_system::acquire(material_properties.texture_maps.at("normal").first);
	}
	else
	{
	    normal_map_ = texture_map::create({});
	    normal_map_->map_texture = texture_system::get_default_normal_texture();
	}
	maps.push_back(normal_map_);

	if (material_properties.texture_maps.contains("albedo"))
	{
	    albedo_map_ = texture_map::create(material_properties.texture_maps.at("albedo").second);
	    albedo_map_->map_texture = texture_system::acquire(material_properties.texture_maps.at("albedo").first);
	}
	else
	{
	    albedo_map_ = texture_map::create({});
	    albedo_map_->map_texture = texture_system::get_default_albedo_texture();
	}
	maps.push_back(albedo_map_);


	if (material_properties.texture_maps.contains("metallic"))
	{
	    metallic_map_ = texture_map::create(material_properties.texture_maps.at("metallic").second);
	    metallic_map_->map_texture = texture_system::acquire(material_properties.texture_maps.at("metallic").first);
	}
	else
	{
	    metallic_map_ = texture_map::create({});
	    metallic_map_->map_texture = texture_system::get_default_metallic_texture();
	}
	maps.push_back(metallic_map_);

	if (material_properties.texture_maps.contains("roughness"))
	{
	    roughness_map_ = texture_map::create(material_properties.texture_maps.at("roughness").second);
	    roughness_map_->map_texture = texture_system::acquire(material_properties.texture_maps.at("roughness").first);
	}
	else
	{
	    roughness_map_ = texture_map::create({});
	    roughness_map_->map_texture = texture_system::get_default_roughness_texture();
	}
	maps.push_back(roughness_map_);

	if (material_properties.texture_maps.contains("ao"))
	{
	    ao_map_ = texture_map::create(material_properties.texture_maps.at("ao").second);
	    ao_map_->map_texture = texture_system::acquire(material_properties.texture_maps.at("ao").first);
	}
	else
	{
	    ao_map_ = texture_map::create({});
	    ao_map_->map_texture = texture_system::get_default_ao_texture();
	}
	maps.push_back(ao_map_);

	auto shader = shader_system::get_shader(shader_name_);
	shader_id_ = shader->get_id();

	set_generation(0);

	for (auto& map : maps)
	{
	    map->acquire();
	}
	internal_id_ = shader->acquire_instance_resources(maps);
    }

    material::~material() { free(); }

    void material::free()
    {
	if (diffuse_map_)
	{
	    diffuse_map_->release();
	}

	if (specular_map_)
	{
	    specular_map_->release();
	}

	if (normal_map_)
	{
	    normal_map_->release();
	}
	if (albedo_map_)
	{
	    albedo_map_->release();
	}
	if (roughness_map_)
	{
	    roughness_map_->release();
	}
	if (metallic_map_)
	{
	    metallic_map_->release();
	}
	if (ao_map_)
	{
	    ao_map_->release();
	}
    }

    void material::set_diffuse_colour(const float4 diffuse) { diffuse_colour_ = diffuse; }
}
