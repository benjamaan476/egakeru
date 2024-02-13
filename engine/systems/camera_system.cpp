#include "camera_system.h"
#include "renderer/renderer_frontend.h"

namespace egkr
{
	static camera_system::unique_ptr camera_system_{};

	bool camera_system::create(const renderer_frontend* renderer_context, const camera_system_configuration& configuration)
	{
		camera_system_ = std::make_unique<camera_system>(renderer_context, configuration);
		return true;
	}

	camera_system::camera_system(const renderer_frontend* renderer_context, const camera_system_configuration& configuration)
		: renderer_context_{ renderer_context }, configuration_{ configuration }
	{}

	bool camera_system::init()
	{
		const auto& configuration = camera_system_->configuration_;

		if (configuration.max_registered_cameras == 0)
		{
			LOG_ERROR("Max cameras must be non-zero");
			return false;
		}

		camera_system_->cameras_.reserve(configuration.max_registered_cameras);
		camera_system_->camera_id_by_name_.reserve(configuration.max_registered_cameras);

		create_default();
		return true;
	}

	bool camera_system::shutdown()
	{
		camera_system_->cameras_.clear();
		camera_system_->camera_id_by_name_.clear();
		return true;
	}

	void camera_system::create_default()
	{
		camera_system_->default_camera_ = camera::create(DEFAULT_CAMERA_NAME);
		camera_system_->default_camera_->set_rotation({ 0.F, 0.F, glm::radians(180.F) });
		camera_system_->default_camera_->get_view();
	}

	camera::shared_ptr camera_system::acquire(std::string_view name)
	{
		if (camera_system_->camera_id_by_name_.contains(name.data()))
		{
			return camera_system_->cameras_[camera_system_->camera_id_by_name_[name.data()]];
		}

		auto id = (uint32_t)camera_system_->cameras_.size();

		camera_system_->cameras_.push_back(camera::create(name));
		camera_system_->camera_id_by_name_[name.data()] = id;

		return camera::shared_ptr();
	}

	void camera_system::release(const camera::shared_ptr /*camera*/)
	{}

	camera::shared_ptr camera_system::get_default()
	{
		return camera_system_->default_camera_;
	}
}