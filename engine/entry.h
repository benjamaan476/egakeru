#pragma once

#include "pch.h"
#include "application/application.h"
#include "game/game.h"

extern egkr::game::unique_ptr create_game();

int main()
{
	auto game = create_game();

	auto application = egkr::application::create(std::move(game));
	application->get_state().game->set_application(application.get());
	application->run();
	application->shutdown();
}