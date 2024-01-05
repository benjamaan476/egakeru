#pragma once

#include "resources/geometry.h"
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
		static void upload_data_range(const vulkan_context* context, vk::CommandPool pool, vk::Fence fence, vk::Queue queue, buffer::shared_ptr buffer, uint64_t offset, uint64_t size, const void* data);
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
