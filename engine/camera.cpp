#include "camera.h"

namespace egkr
{
	camera::shared_ptr camera::create(const camera_properties& properties)
	{
		return std::make_shared<camera>(properties);
	}

	camera::camera(const camera_properties& properties)
	{
	}

	camera::~camera()
	{
		destroy();
	}

	camera::destroy()
	{

	}
}