#pragma once

#include <pch.h>

namespace egkr
{
	class renderer_backend;

	namespace renderbuffer
	{
		enum class type
		{
			unknown,
			vertex,
			index,
			uniform,
			staging,
			read,
			storage
		};

		class renderbuffer
		{
		public:
			using shared_ptr = std::shared_ptr<renderbuffer>;
			static shared_ptr create(const renderer_backend* backend, type bffer_type, uint64_t size);

			renderbuffer(const renderer_backend* backend, type buffer_type, uint64_t size);
			virtual ~renderbuffer() = default;

			virtual void bind(uint64_t offset) = 0;
			virtual void unbind() = 0;

			virtual void* map_memory(uint64_t offset, uint64_t size) = 0;
			virtual void unmap() = 0;

			virtual void flush(uint64_t offset, uint64_t size) = 0;

			virtual void read(uint64_t offset, uint64_t size, void* out) = 0;
			virtual void resize(uint64_t new_size) = 0;

			virtual void load_range(uint64_t offset, uint64_t size, const void* data) = 0;
			virtual void copy_range(uint64_t source_offset, renderbuffer* dest, uint64_t dest_offset, uint64_t size) = 0;

			virtual void draw(uint64_t offset, uint32_t element_count, bool bind_only) = 0;

			virtual void* get_buffer() = 0;

		protected:
			type type_{};
			const renderer_backend* backend_{};
			uint64_t total_size_{};
		};
	}
}
