#pragma once

#include "game/game.h"

//TODO Hack
#include "renderer/renderer_frontend.h"

class sandbox_game final : public egkr::game
{
public:
	explicit sandbox_game(const egkr::application_configuration& configuration);
	bool init() final;
	void update(double delta_time) final;
	void render(double delta_time) final;
	void resize(uint32_t width, uint32_t height) final;

private:
	void recalculate_view_matrix();
	void camera_yaw(float amount);
private:
	bool view_dirty{true};
	egkr::float3 position_{0, 0, 30};
	egkr::float3 rotation_{};
	egkr::float4x4 view_{};
};