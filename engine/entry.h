#pragma once

#include "pch.h"
#include "application/application.h"
#include "game/game.h"

extern egkr::game::unique_ptr create_game();

int main()
{
	auto game = create_game();

	egkr::application::create(std::move(game));
	egkr::application::run();
	egkr::application::shutdown();
}