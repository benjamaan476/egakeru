#pragma once

#include "resources/geometry.h"
#include <renderer/renderbuffer.h>
#include "buffer.h"

namespace egkr::geometry
{
	class vulkan_geometry : public geometry
	{
	public:
		static shared_ptr create(const renderer_backend* backend, const vulkan_context* context, const properties& properties);

		vulkan_geometry(const renderer_backend* backend, const vulkan_context* context, const properties& properties);
		~vulkan_geometry() override;

		bool populate(const properties& properties) override;
		void draw() override;
		void update_vertices(uint32_t offset, uint32_t vertex_count, void* vertices) override;
		void free() override;
	private:
		const vulkan_context* context_{};

		renderbuffer::renderbuffer::shared_ptr vertex_buffer_{};
		uint32_t vertex_count_{};
		uint32_t vertex_size_{};
		uint32_t vertex_offset_{};

		renderbuffer::renderbuffer::shared_ptr index_buffer_{};
		uint32_t index_count_{};
		uint32_t index_size_{};
		uint32_t index_offset_{};
	};

}
