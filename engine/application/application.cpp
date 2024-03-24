#include "application.h"

#include "engine/engine.h"

namespace egkr
{
	application::application(engine_configuration configuration)
		: engine_configuration_{ std::move(configuration) }
	{
		width_ = engine_configuration_.width;
		height_ = engine_configuration_.height;
	}

	void application::set_engine(engine* engine)
	{
		engine_ = engine;
	}
}