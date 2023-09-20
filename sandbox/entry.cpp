#include "pch.h"

#include "entry.h"
#include "sandbox_game.h"



egkr::game::unique_ptr create_game()
{
	const uint32_t width{ 800 };
	const uint32_t height{ 600 };
	const std::string name{"sandbox"};
	const auto application_config = egkr::application_configuration{ .width = width, .height = height, .name = name };

	auto game = std::make_unique<sandbox_game>(application_config);
	return game;
}