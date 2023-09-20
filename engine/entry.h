#pragma once

#include "pch.h"
#include "application/application.h"
#include "game/game.h"

extern egkr::game::unique_ptr create_game();

int main()
{
	auto game = create_game();

	egkr::log::init();
	LOG_TRACE("Hello");
	LOG_INFO("Hello");
	LOG_ERROR("Hello");
	LOG_WARN("Hello");
	LOG_FATAL("Hello");

	auto application = egkr::application::create(std::move(game));

	application->run();

}