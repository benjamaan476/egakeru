#include "renderbuffer.h"

#include <renderer/renderer_types.h>

namespace egkr::renderbuffer
{
	renderbuffer::shared_ptr renderbuffer::create(const renderer_backend* backend, type buffer_type, uint64_t size)
	{
		return backend->create_renderbuffer(buffer_type, size);
	}

	renderbuffer::renderbuffer(const renderer_backend* backend, type buffer_type, uint64_t size)
		: type_{ buffer_type }, backend_{ backend }, total_size_{ size }
	{

	}

}