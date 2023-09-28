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

void sandbox_game::update(std::chrono::milliseconds /*delta_time*/)
{
}

void sandbox_game::render(std::chrono::milliseconds /*delta_time*/)
{
}

void sandbox_game::resize(uint32_t /*width*/, uint32_t /*height*/) {}
