#include "vulkan_geometry.h"
#include "vulkan_types.h"


namespace egkr
{
	void upload_data_range(const vulkan_context* context, vk::CommandPool pool, vk::Fence fence, vk::Queue queue, buffer::shared_ptr buffer, uint64_t offset, uint64_t size, const void* data)
	{
		const vk::MemoryPropertyFlags memory_flags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
		const vk::BufferUsageFlags usage = vk::BufferUsageFlagBits::eTransferSrc;

		auto staging_buffer = buffer::create(context, size, usage, memory_flags, true);

		staging_buffer->load_data(offset, size, 0, data);
		staging_buffer->copy_to(pool, fence, queue, staging_buffer->get_handle(), 0, buffer->get_handle(), 0, size);
	}

	vulkan_geometry::vulkan_geometry(const vulkan_context* context, const geometry_properties& properties)
		: geometry(properties), context_{ context }
	{
		const vk::MemoryPropertyFlags flags{vk::MemoryPropertyFlagBits::eDeviceLocal};
		const vk::BufferUsageFlags usage{vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc};
		const auto vertex_buffer_size = sizeof(vertex_3d) * 1024 * 1024;

		vertex_buffer_ = buffer::create(context_, vertex_buffer_size, usage, flags, true);

		const vk::BufferUsageFlags index_usage{vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc};
		const auto index_buffer_size = sizeof(uint32_t) * 1024 * 1024;

		index_count_ = properties.indices.size();
		index_buffer_ = buffer::create(context_, index_buffer_size, index_usage, flags, true);
		vertex_count_ = properties.vertices.size();
		upload_data_range(context, context->device.graphics_command_pool, VK_NULL_HANDLE, context_->device.graphics_queue, vertex_buffer_, 0, properties.vertices.size() * sizeof(vertex_3d), properties.vertices.data());
		upload_data_range(context, context->device.graphics_command_pool, VK_NULL_HANDLE, context_->device.graphics_queue, index_buffer_, 0, properties.indices.size() * sizeof(uint32_t), properties.indices.data());

	}

	void vulkan_geometry::draw() const
	{
		auto& command_buffer = context_->graphics_command_buffers[context_->image_index];
		vk::DeviceSize offset{ 0 };
		command_buffer.get_handle().bindVertexBuffers(0, vertex_buffer_->get_handle(), offset);

		command_buffer.get_handle().bindIndexBuffer(index_buffer_->get_handle(), offset, vk::IndexType::eUint32);

		if (index_count_)
		{
			command_buffer.get_handle().drawIndexed(index_count_, 1, 0, 0, 0);
		}
		else
		{
			command_buffer.get_handle().draw(vertex_count_, 0, 0, 0);
		}
	}
}