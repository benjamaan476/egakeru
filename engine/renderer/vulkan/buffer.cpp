#include "buffer.h"

#include "vulkan_types.h"

namespace egkr
{
	buffer::shared_ptr buffer::create(const vulkan_context* context, uint64_t size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memory_flags, bool bind_on_create)
	{
		return std::make_shared<buffer>(context, size, usage, memory_flags, bind_on_create);
	}

	buffer::buffer(const vulkan_context* context, uint64_t size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memory_flags, bool bind_on_create)
		:context_{ context }, usage_{ usage }, size_{ size }, memory_property_flags_{ memory_flags }
	{
		const auto& device = context_->device.logical_device;

		vk::BufferCreateInfo create_info{};
		create_info
			.setSize(size_)
			.setUsage(usage_)
			.setSharingMode(vk::SharingMode::eExclusive);

		handle_ = device.createBuffer(create_info, context_->allocator);

		const auto memory_requirements = device.getBufferMemoryRequirements(handle_);

		const auto memory_index = context_->device.find_memory_index(memory_requirements.memoryTypeBits, memory_flags);
		vk::MemoryAllocateInfo alloc_info{};

		alloc_info
			.setAllocationSize(memory_requirements.size)
			.setMemoryTypeIndex(memory_index);
		memory_ = device.allocateMemory(alloc_info, context_->allocator);

		if (bind_on_create)
		{
			bind(0);
		}
	}

	buffer::~buffer()
	{
		destroy();
	}

	void buffer::destroy()
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

	void* egkr::buffer::lock(uint64_t offset, uint64_t size, uint32_t flags)
	{
		auto data = context_->device.logical_device.mapMemory(memory_, offset, size,(vk::MemoryMapFlags)flags);
		return data;
	}

	void buffer::unlock()
	{
		context_->device.logical_device.unmapMemory(memory_);
	}

	void buffer::load_data(uint64_t offset, uint64_t size, uint32_t flags, const void* data)
	{
		auto* dest_data = lock(offset, size, flags);
		std::memcpy(dest_data, data, size);
		unlock();
	}

	void buffer::copy_to(vk::CommandPool pool, vk::Fence /*fence*/, vk::Queue queue, vk::Buffer source, uint64_t source_offset, vk::Buffer dest, uint64_t dest_offset, uint64_t size)
	{
		context_->device.logical_device.waitIdle();
		command_buffer command_buffer{};

		command_buffer.begin_single_use(context_, pool);

		const vk::BufferCopy buffer_copy(source_offset, dest_offset, size);
		command_buffer.get_handle().copyBuffer(source, dest, buffer_copy);

		command_buffer.end_single_use(context_, pool, queue);
	}

	void buffer::resize(uint64_t new_size, vk::Queue queue, vk::CommandPool pool)
	{
		const auto& device = context_->device.logical_device;

		vk::BufferCreateInfo new_create_info{};
		new_create_info
			.setSize(new_size)
			.setUsage(usage_)
			.setSharingMode(vk::SharingMode::eExclusive);

		auto new_buffer = device.createBuffer(new_create_info, context_->allocator);
		const auto memory_requirements = device.getBufferMemoryRequirements(new_buffer);

		const auto memory_index = context_->device.find_memory_index(memory_requirements.memoryTypeBits, memory_property_flags_);
		vk::MemoryAllocateInfo alloc_info{};

		alloc_info
			.setAllocationSize(memory_requirements.size)
			.setMemoryTypeIndex(memory_index);
		auto new_memory = device.allocateMemory(alloc_info, context_->allocator);

		device.bindBufferMemory(new_buffer, new_memory, 0);

		copy_to(pool, VK_NULL_HANDLE, queue, handle_, 0, new_buffer, 0, new_size);

		if (memory_)
		{
			device.freeMemory(memory_, context_->allocator);
		}

		if (handle_)
		{
			device.destroyBuffer(handle_, context_->allocator);
		}

		size_ = new_size;
		handle_ = new_buffer;
		memory_ = new_memory;
	}

	void buffer::bind(uint64_t offset)
	{
		context_->device.logical_device.bindBufferMemory(handle_, memory_, offset);
	}
}