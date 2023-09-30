#include "game.h"

#include "application/application.h"

namespace egkr
{
	game::game(application_configuration configuration)
		: application_configuration_{ std::move(configuration) }
	{

	}

	void game::set_application(application* app)
	{
		application_ = app;
	}
}