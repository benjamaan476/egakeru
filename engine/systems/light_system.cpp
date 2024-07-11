#include <systems/light_system.h>

namespace egkr
{

	static light_system::unique_ptr light_system_{};

	light_system* light_system::create()
	{
		light_system_ = std::make_unique<light_system>();
		return light_system_.get();
	}

	light_system::light_system()
		: max_point_light_count_{ 10 }
	{}

	light_system::~light_system()
	{
		shutdown();
	}

	bool light_system::init()
	{
		return true;
	}

	bool light_system::shutdown()
	{
		point_lights_.clear();
		directional_light_.reset();

		light_system_ = nullptr;
		return true;
	}

	bool light_system::add_directional_light(const std::shared_ptr<light::directional_light>& light)
	{
		if (!light_system_->directional_light_)
		{
			light_system_->directional_light_ = light;
			return true;
		}
		LOG_WARN("Directional light already registered. Remove before adding a new one");
		return false;
	}

	bool light_system::remove_directional_light()
	{
		if (light_system_->directional_light_)
		{
			light_system_.reset();
			return true;
		}

		LOG_WARN("Tried to remove a directional light that isn't registered");
		return false;
	}

	light_system::light_reference light_system::add_point_light(const light::point_light& light)
	{
		uint32_t reference = (uint32_t)light_system_->point_lights_.size();
		if (reference + 1 == light_system_->max_point_light_count_)
		{
			LOG_ERROR("Max point light count exceeded. Light not added");
			return false;
		}

		light_system_->point_lights_.push_back(light);
		return reference;
	}

	bool light_system::remove_point_light(light_reference /*light*/)
	{
		LOG_WARN("Cannot remove point lights just yet. Come back later.");
		return false;
	}

	int32_t light_system::point_light_count()
	{
		return (int32_t)light_system_->point_lights_.size();
	}

	light::directional_light* light_system::get_directional_light()
	{
		return light_system_->directional_light_.get();
	}
	const std::vector<light::point_light>& light_system::get_point_lights()
	{
		return light_system_->point_lights_;
	}
}
