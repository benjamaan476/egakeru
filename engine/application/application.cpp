#include "application.h"
#include "engine/engine.h"

namespace egkr
{
	application::application(engine_configuration configuration, renderer_backend::unique_ptr renderer_plugin)
		: engine_configuration_{ std::move(configuration) }, renderer_plugin{std::move(renderer_plugin)}
	{
		width_ = engine_configuration_.width;
		height_ = engine_configuration_.height;
	}

	void application::set_engine(engine* engine)
	{
		engine_ = engine;
	}

	const renderer_frontend::unique_ptr& application::get_renderer() const
	{
		return engine_->get_renderer(); 
	}
}
