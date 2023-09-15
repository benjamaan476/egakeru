#include <iostream>

#include "pch.h"

#include "application/application.h"

int main()
{
	std::cout << "FK" << std::endl;

	egkr::log::init();
	LOG_TRACE("Hello");
	LOG_INFO("Hello");
	LOG_ERROR("Hello");
	LOG_WARN("Hello");
	LOG_FATAL("Hello");

	const uint32_t width{ 800 };
	const uint32_t height{ 600 };
	const std::string name{"sandbox"};
	const auto application_config = egkr::application_configuration{ .width = width, .height = height, .name = name };

	auto application = egkr::application::create(application_config);

	application->run();

}
