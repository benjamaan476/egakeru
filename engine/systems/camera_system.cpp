#include "camera_system.h"
#include "renderer/renderer_frontend.h"

namespace egkr
{
	static camera_system::unique_ptr camera_system_{};

	camera_system* camera_system::create(const configuration& configuration)
	{
		camera_system_ = std::make_unique<camera_system>(configuration);
		return camera_system_.get();
	}

	camera_system::camera_system(const configuration& configuration)
		: configuration_{ configuration }
	{
		create_default();
	}

	bool camera_system::init()
	{
		if (configuration_.max_registered_cameras == 0)
		{
			LOG_ERROR("Max cameras must be non-zero");
			return false;
		}

		cameras_.reserve(configuration_.max_registered_cameras);
		camera_id_by_name_.reserve(configuration_.max_registered_cameras);

		return true;
	}

	bool camera_system::shutdown()
	{
		cameras_.clear();
		camera_id_by_name_.clear();
		return true;
	}

	void camera_system::create_default()
	{
		default_camera_ = camera::create(DEFAULT_CAMERA_NAME);
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
