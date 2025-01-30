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
}
