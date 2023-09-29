#pragma once
#include "pch.h"

#include <vulkan/vulkan.hpp>

namespace egkr
{
	struct vulkan_context;
	class buffer
	{
	public:
		using shared_ptr = std::shared_ptr<buffer>;
		static shared_ptr create(const vulkan_context* context, uint64_t size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memory_flags, bool bind_on_create);

		buffer(const vulkan_context* context, uint64_t size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memory_flags, bool bind_on_create);
		~buffer();

		void destroy();

		void* lock(uint64_t offset, uint64_t size, uint32_t flags);
		void unlock();

		void load_data(uint64_t offset, uint64_t size, uint32_t flags, const void* data);

		void copy_to(vk::CommandPool pool, vk::Fence fence, vk::Queue queue, vk::Buffer source, uint64_t source_offset, vk::Buffer dest, uint64_t dest_offset, uint64_t size);
		void resize(uint64_t new_size, vk::Queue queue, vk::CommandPool pool);

		void bind(uint64_t offset);

		const auto& get_handle() const { return handle_; }

	private:
		const vulkan_context* context_{};

		vk::Buffer handle_{};
		vk::BufferUsageFlags usage_{};
		vk::DeviceMemory memory_{};

		uint64_t size_{};
		uint32_t memory_index_{};
		vk::MemoryPropertyFlags memory_property_flags_{};

		bool is_locked_{};
	};
}
