#include "platform.h"

#include "windows/platform_windows.h"
namespace egkr
{
	platform::shared_ptr platform::create(platform_type type)
	{
		switch (type)
		{
		case platform_type::windows:
			return platform_windows::create();
		case platform_type::linux:
		case platform_type::macos:
		case platform_type::android:
		default:
			LOG_ERROR("Unsupported platform type: {0}", platform_type_to_string(type));
			return nullptr;
		}
	}
}
