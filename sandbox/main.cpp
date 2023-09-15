#include <iostream>

#include "log/log.h"
#include "platform/platform.h"
int main()
{
	std::cout << "FK" << std::endl;

	egkr::log::init(); 
	LOG_TRACE("Hello");
	LOG_INFO("Hello");
	LOG_ERROR("Hello");
	LOG_WARN("Hello"); 
	LOG_FATAL("Hello"); 
	
	const uint32_t width = 800;
	const uint32_t height = 600;
	const uint32_t start_x = 100;
	const uint32_t start_y = 100;
	auto platform_configuration = egkr::platform_configuration{ .start_x = start_x, .start_y = start_y, .width = width, .height = height };
	auto platform = egkr::platform::create(egkr::platform_type::windows);
	platform->startup(platform_configuration);

	while (platform->is_running())
	{
		platform->pump();
	}

}
