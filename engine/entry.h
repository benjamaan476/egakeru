#pragma once

#include "pch.h"
#include "engine/engine.h"
#include "application/application.h"

class renderer_frontend;
extern egkr::application::unique_ptr create_application();
extern void initialise_renderer_backend(egkr::renderer_frontend* frontend);

int main()
{
	auto application = create_application();

	egkr::engine::create(std::move(application), initialise_renderer_backend);
	egkr::engine::run();
	egkr::engine::shutdown();
}