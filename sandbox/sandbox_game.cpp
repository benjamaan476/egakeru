#include "sandbox_game.h"

sandbox_game::sandbox_game(const egkr::application_configuration& configuration)
	: game(configuration)
{
}

bool sandbox_game::init()
{
	LOG_INFO("Sandbox game created");

	return true;
}

void sandbox_game::update()
{
}

void sandbox_game::render()
{
}

void sandbox_game::resize()
{
}
