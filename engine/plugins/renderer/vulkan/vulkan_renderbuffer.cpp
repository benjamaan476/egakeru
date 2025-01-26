#include "vulkan_renderbuffer.h"

#include "vulkan_types.h"
#include "renderer_vulkan.h"

namespace egkr
{
	vulkan_buffer::shared_ptr vulkan_buffer::create(const vulkan_context* context, egkr::renderbuffer::type buffer_type, uint64_t size)
	{
		return std::make_shared<vulkan_buffer>(context, buffer_type, size);
	}

	vulkan_buffer::vulkan_buffer(const vulkan_context* context, egkr::renderbuffer::type buffer_type, uint64_t size)
		: renderbuffer{ buffer_type, size }, context_{ context }
	{
		std::string buffer_name{};
		switch (buffer_type)
		{
			using enum egkr::renderbuffer::type;
		case vertex:
			usage_ = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc;
			memory_property_flags_ = vk::MemoryPropertyFlagBits::eDeviceLocal;
			buffer_name = "_vertex_";
			break;
		case index:
			usage_ = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc;
			memory_property_flags_ = vk::MemoryPropertyFlagBits::eDeviceLocal;
			buffer_name = "_index_";
			break;
		case uniform:
			usage_ = vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst;
			memory_property_flags_ = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eDeviceLocal;
			buffer_name = "_uniform_";
			break;
		case staging:
			usage_ = vk::BufferUsageFlagBits::eTransferSrc;
			memory_property_flags_ = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
			buffer_name = "_staging_";
			break;
		case read:
			usage_ = vk::BufferUsageFlagBits::eTransferDst;
			memory_property_flags_ = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
			buffer_name = "_read_";
			break;
		case storage:
			LOG_ERROR("Storage buffers not supported yet");
			return;
		default:
			LOG_ERROR("Unknown buffer type specified");
			return;
		}

		vk::BufferCreateInfo create_info{};
		create_info
			.setSize(size)
			.setUsage(usage_)
			.setSharingMode(vk::SharingMode::eExclusive);

		handle_ = context_->device.logical_device.createBuffer(create_info, context_->allocator);

		memory_requirements_ = context_->device.logical_device.getBufferMemoryRequirements(handle_);
		memory_index_ = context_->device.find_memory_index(memory_requirements_.memoryTypeBits, memory_property_flags_);

		SET_DEBUG_NAME(context_, VkObjectType::VK_OBJECT_TYPE_BUFFER, (uint64_t)(VkBuffer)handle_, "buffer" + buffer_name)

		if (memory_index_ == invalid_32_id)
		{
			LOG_ERROR("Unable to create buffer because the required memory index was not found");
			return;
		}

		vk::MemoryAllocateInfo allocate_info{};
		allocate_info
			.setAllocationSize(memory_requirements_.size)
			.setMemoryTypeIndex(memory_index_);

		memory_ = context_->device.logical_device.allocateMemory(allocate_info, context_->allocator);

	}

	vulkan_buffer::~vulkan_buffer()
	{
		destroy();
	}

	void vulkan_buffer::destroy()
	{
		if (memory_)
		{
			context_->device.logical_device.freeMemory(memory_, context_->allocator);
			memory_ = VK_NULL_HANDLE;
		}

		if (handle_)
		{
			context_->device.logical_device.destroyBuffer(handle_, context_->allocator);
			handle_ = VK_NULL_HANDLE;
		}
	}



	void vulkan_buffer::bind(uint64_t offset)
	{
		context_->device.logical_device.bindBufferMemory(handle_, memory_, offset);
	}

	void vulkan_buffer::unbind()
	{
	}

	void* vulkan_buffer::map_memory(uint64_t offset, uint64_t size)
	{
		return context_->device.logical_device.mapMemory(memory_, offset, size);
	}

	void vulkan_buffer::unmap()
	{
		context_->device.logical_device.unmapMemory(memory_);
	}

	void vulkan_buffer::flush(uint64_t offset, uint64_t size)
	{
		if (!is_host_coherent())
		{
			vk::MappedMemoryRange range{};
			range
				.setMemory(memory_)
				.setOffset(offset)
				.setSize(size);

			context_->device.logical_device.flushMappedMemoryRanges(range);
		}
	}

	void vulkan_buffer::read(uint64_t offset, uint64_t size, void* out)
	{
		if (is_device_local() && !is_host_visible())
		{
			//Create staging read buffer then read from that
			auto read_buffer = renderbuffer::renderbuffer::create(egkr::renderbuffer::type::read, size);
			read_buffer->bind(0);

			copy_range(offset, read_buffer.get(), 0, size);

			auto mapped_data = read_buffer->map_memory(0, size);
			memcpy(out, mapped_data, size);
			read_buffer->unmap();

			read_buffer->unbind();
		}
		else
		{
			auto mapped_data = map_memory(offset, size);
			memcpy(out, mapped_data, size);
			unmap();
		}

	}

	void vulkan_buffer::resize(uint64_t new_size)
	{
		vk::BufferCreateInfo buffer_info{};
		buffer_info
			.setSize(new_size)
			.setUsage(usage_)
			.setSharingMode(vk::SharingMode::eExclusive);

		vk::Buffer new_buffer = context_->device.logical_device.createBuffer(buffer_info, context_->allocator);
		auto memory_requirements = context_->device.logical_device.getBufferMemoryRequirements(new_buffer);

		vk::MemoryAllocateInfo allocate_info{};
		allocate_info
			.setAllocationSize(memory_requirements.size)
			.setMemoryTypeIndex(memory_index_);

		vk::DeviceMemory new_memory = context_->device.logical_device.allocateMemory(allocate_info, context_->allocator);

		context_->device.logical_device.bindBufferMemory(new_buffer, new_memory, 0);
		copy_range(0, new_buffer, 0, total_size_);

		context_->device.logical_device.waitIdle();

		destroy();

		memory_requirements_ = memory_requirements;
		memory_ = new_memory;
		handle_ = new_buffer;
	}

	void vulkan_buffer::load_range(uint64_t offset, uint64_t size, const void* data)
	{
		if (is_device_local() && !is_host_visible())
		{
			//Create a staging buffer and load into that 
			auto staging = renderbuffer::renderbuffer::create(egkr::renderbuffer::type::staging, size);
			staging->bind(0);

			staging->load_range(0, size, data);
			staging->copy_range(0, this, offset, size);

			staging->unbind();
		}
		else
		{
			auto mapped_data = map_memory(offset, size);
			memcpy(mapped_data, data, size);
			unmap();
		}
	}

	void vulkan_buffer::copy_range(uint64_t source_offset, egkr::renderbuffer::renderbuffer* destination, uint64_t dest_offset, uint64_t size)
	{
		copy_range(source_offset, *((vk::Buffer*)destination->get_buffer()), dest_offset, size);
	}

	void vulkan_buffer::draw(uint64_t offset, uint32_t element_count, bool bind_only)
	{
		auto& command_buffer = context_->graphics_command_buffers[context_->image_index];

		if (type_ == egkr::renderbuffer::type::vertex)
		{
			command_buffer.get_handle().bindVertexBuffers(0, handle_, offset);
			if (!bind_only)
			{
				command_buffer.get_handle().draw(element_count, 1, 0, 0);
			}
		}
		else if (type_ == egkr::renderbuffer::type::index)
		{
			command_buffer.get_handle().bindIndexBuffer(handle_, offset, vk::IndexType::eUint32);
			if (!bind_only)
			{
				command_buffer.get_handle().drawIndexed(element_count, 1, 0, 0, 0);
			}
		}
		else
		{
			LOG_ERROR("Cannot draw with provided buffer type");
		}
	}

	void* vulkan_buffer::get_buffer()
	{
		return &handle_;
	}

	bool vulkan_buffer::is_device_local() const
	{
		return (memory_property_flags_ & vk::MemoryPropertyFlagBits::eDeviceLocal) == vk::MemoryPropertyFlagBits::eDeviceLocal;
	}

	bool vulkan_buffer::is_host_visible() const
	{
		return (memory_property_flags_ & vk::MemoryPropertyFlagBits::eHostVisible) == vk::MemoryPropertyFlagBits::eHostVisible;
	}
	
	bool vulkan_buffer::is_host_coherent() const
	{
		return (memory_property_flags_ & vk::MemoryPropertyFlagBits::eHostCoherent) == vk::MemoryPropertyFlagBits::eHostVisible;
	}
	void vulkan_buffer::copy_range(uint64_t source_offset, vk::Buffer destination, uint64_t dest_offset, uint64_t size)
	{
		vk::Queue queue = context_->device.graphics_queue;
		queue.waitIdle();

		command_buffer temp_command{};
		temp_command.begin_single_use(context_, context_->device.graphics_command_pool);

		vk::BufferCopy copy_region;
		copy_region
			.setSrcOffset(source_offset)
			.setDstOffset(dest_offset)
			.setSize(size);

		temp_command.get_handle().copyBuffer(handle_, destination, copy_region);

		temp_command.end_single_use(context_, context_->device.graphics_command_pool, queue);
	}
}
