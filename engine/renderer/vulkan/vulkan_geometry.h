#pragma once

#include "resources/geometry.h"
#include "buffer.h"

namespace egkr
{
	struct vulkan_geometry_state
	{
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

	inline static void upload_data_range(const vulkan_context* context, vk::CommandPool pool, vk::Fence fence, vk::Queue queue, buffer::shared_ptr buffer, uint64_t offset, uint64_t size, const void* data)
	{
		const vk::MemoryPropertyFlags memory_flags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
		const vk::BufferUsageFlags usage = vk::BufferUsageFlagBits::eTransferSrc;

		auto staging_buffer = buffer::create(context, size, usage, memory_flags, true);

		staging_buffer->load_data(offset, size, 0, data);
		staging_buffer->copy_to(pool, fence, queue, staging_buffer->get_handle(), 0, buffer->get_handle(), 0, size);
	}
}
