#include "renderbuffer.h"

#include <renderer/renderer_frontend.h>

namespace egkr::renderbuffer
{
	renderbuffer::shared_ptr renderbuffer::create(type buffer_type, uint64_t size)
	{
		return renderer->create_renderbuffer(buffer_type, size);
	}

	renderbuffer::renderbuffer(type buffer_type, uint64_t size)
		:  total_size_{ size }, type_{ buffer_type }
	{

	}
}
