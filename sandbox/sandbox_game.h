#pragma once

#include "game/game.h"

class sandbox_game final : public egkr::game
{
public:
	explicit sandbox_game(const egkr::application_configuration& configuration);
	bool init() final;
	void update(std::chrono::milliseconds delta_time) final;
	void render(std::chrono::milliseconds delta_time) final;
	void resize(uint32_t width, uint32_t height) final;
};