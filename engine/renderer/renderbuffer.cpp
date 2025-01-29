#include "renderbuffer.h"

#include "engine/engine.h"

namespace egkr::renderbuffer
{
	renderbuffer::shared_ptr renderbuffer::create(type buffer_type, uint64_t size)
	{
		return engine::get()->get_renderer()->create_renderbuffer(buffer_type, size);
	}

	renderbuffer::renderbuffer(type buffer_type, uint64_t size)
		:  total_size_{ size }, type_{ buffer_type }
	{

	}
}
