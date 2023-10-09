#pragma once

#include "resources/geometry.h"
#include "buffer.h"

namespace egkr
{
	struct vulkan_context;
	class vulkan_geometry : public geometry
	{
	public:
		explicit vulkan_geometry(const vulkan_context* context, const geometry_properties& properties);
		~vulkan_geometry() override;

		void draw() const override;
	private:
		const vulkan_context* context_{};

		egkr::buffer::shared_ptr vertex_buffer_{};
		uint32_t vertex_count_{};
		uint32_t vertex_size_{};
		uint32_t vertex_offset_{};

		egkr::buffer::shared_ptr index_buffer_{};
		uint32_t index_count_{};
		uint32_t index_size_{};
		uint32_t index_offset_{};
};
}
