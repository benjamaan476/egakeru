#pragma once

#include "game/game.h"
#include "systems/camera_system.h"

class sandbox_game final : public egkr::game
{
public:
	explicit sandbox_game(const egkr::application_configuration& configuration);
	bool init() final;
	void update(double delta_time) final;
	void render(double delta_time) final;
	void resize(uint32_t width, uint32_t height) final;

private:
	egkr::camera::shared_ptr camera_{};
};