#pragma once

#include "pch.h"
#include "engine/engine.h"
#include "application/application.h"

extern egkr::application::unique_ptr create_application();

int main()
{
	auto application = create_application();

	egkr::engine::create(std::move(application));
	egkr::engine::run();
	egkr::engine::shutdown();
}