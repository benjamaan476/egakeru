#include "platform.h"

#ifdef WIN32
#include "windows/platform_windows.h"
#elif LINUX
#include "linux/platform_linux.h"
#endif
namespace egkr
{
	platform::shared_ptr platform::create(const platform::configuration& configuration)
	{
		return internal_platform::create(configuration);
	}

	std::optional<platform::dynamic_library> platform::load_library(const std::string& library_name)
	{
		return internal_platform::load_library(library_name);
	}

	bool platform::unload_library(dynamic_library& library)
	{
		return internal_platform::unload_library(library);
	}

	bool platform::load_function(const std::string& function_name, dynamic_library& library)
	{
		return internal_platform::load_function(function_name, library);
	}


}
