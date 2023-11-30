#include "vulkan_geometry.h"
#include "renderer/renderer_types.h"
#include "vulkan_types.h"

namespace egkr::geometry
{
	geometry::shared_ptr vulkan_geometry::create(const renderer_backend* backend, const vulkan_context* context, const properties& properties)
	{
		auto geom = std::make_shared<vulkan_geometry>(backend, context, properties);

		if (properties.vertex_count)
		{
			geom->populate(properties);
		}
		else
		{
			LOG_WARN("Geometry has no vertex data. Not populating");
		}
		return geom;
	}

	vulkan_geometry::vulkan_geometry(const renderer_backend* backend, const vulkan_context* context, const properties& properties)
		: geometry(backend, properties), context_{context}
	{}

	vulkan_geometry::~vulkan_geometry()
	{
		free();
	}

	bool vulkan_geometry::populate(const properties & properties)
	{
		ZoneScoped;

		const vk::MemoryPropertyFlags flags{vk::MemoryPropertyFlagBits::eDeviceLocal};
		const vk::BufferUsageFlags usage{vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc};
		const auto vertex_buffer_size = properties.vertex_size * properties.vertex_count;

		vertex_count_ = properties.vertex_count;
		vertex_buffer_ = buffer::create(context_, vertex_buffer_size, usage, flags, true);

		const vk::BufferUsageFlags index_usage{vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc};
		const auto index_buffer_size = sizeof(uint32_t) * properties.indices.size();

		index_count_ = properties.indices.size();
		index_buffer_ = buffer::create(context_, index_buffer_size, index_usage, flags, true);
		upload_data_range(context_, context_->device.graphics_command_pool, VK_NULL_HANDLE, context_->device.graphics_queue, vertex_buffer_, 0, vertex_buffer_size, properties.vertices);
		upload_data_range(context_, context_->device.graphics_command_pool, VK_NULL_HANDLE, context_->device.graphics_queue, index_buffer_, 0, index_buffer_size, properties.indices.data());

		return true;
	}

	void vulkan_geometry::draw()
	{
		ZoneScoped;

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

	void vulkan_geometry::free()
	{
		ZoneScoped;

		if (context_)
		{
			context_->device.logical_device.waitIdle();

			backend_->free_material(material_.get());

			if (vertex_buffer_)
			{
				vertex_buffer_->destroy();
				vertex_buffer_.reset();
			}

			if (index_buffer_)
			{
				index_buffer_->destroy();
				index_buffer_.reset();
			}
		}
	}

	void vulkan_geometry::upload_data_range(const vulkan_context* context, vk::CommandPool pool, vk::Fence fence, vk::Queue queue, buffer::shared_ptr buffer, uint64_t offset, uint64_t size, const void* data)
	{
		ZoneScoped;

		const vk::MemoryPropertyFlags memory_flags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
		const vk::BufferUsageFlags usage = vk::BufferUsageFlagBits::eTransferSrc;

		auto staging_buffer = buffer::create(context, size, usage, memory_flags, true);

		staging_buffer->load_data(offset, size, 0, data);
		staging_buffer->copy_to(pool, fence, queue, staging_buffer->get_handle(), 0, buffer->get_handle(), 0, size);
	}
}