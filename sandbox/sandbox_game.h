#pragma once

#include "game/game.h"

class sandbox_game final : public egkr::game
{
public:
	explicit sandbox_game(const egkr::application_configuration& configuration);
	bool init() final;
	void update() final;
	void render() final;
	void resize() final;
};