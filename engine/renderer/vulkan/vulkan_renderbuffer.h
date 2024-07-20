#pragma once
#include "pch.h"

#include <renderer/renderbuffer.h>
#include <vulkan/vulkan.hpp>

namespace egkr
{
	struct vulkan_context;
	class vulkan_buffer : public renderbuffer::renderbuffer
	{
	public:
		using shared_ptr = std::shared_ptr<vulkan_buffer>;
		static shared_ptr create(const vulkan_context* context, egkr::renderbuffer::type buffer_type, uint64_t size);

		vulkan_buffer(const vulkan_context* context, egkr::renderbuffer::type buffer_type, uint64_t size);
		~vulkan_buffer() override;

		void destroy();

		void bind(uint64_t offset) override;
		void unbind() override;

		void* map_memory(uint64_t offset, uint64_t size) override;
		void unmap() override;

		void flush(uint64_t offset, uint64_t size) override;

		void read(uint64_t offset, uint64_t size, void* out) override;
		void resize(uint64_t new_size) override;

		void load_range(uint64_t offset, uint64_t size, const void* data) override;
		void copy_range(uint64_t source_offset, egkr::renderbuffer::renderbuffer* destination, uint64_t dest_offset, uint64_t size) override;

		void draw(uint64_t offset, uint32_t element_count, bool bind_only) override;

		void* get_buffer() override;

		const auto& get_handle() const { return handle_; }

		uint64_t get_size() const override { return total_size_; }

		bool is_device_local() const;
		bool is_host_visible() const;
		bool is_host_coherent() const;

	protected:
		void copy_range(uint64_t source_offset, vk::Buffer destination, uint64_t dest_offset, uint64_t size);
	private:
		const vulkan_context* context_{};

		vk::Buffer handle_{};
		vk::BufferUsageFlags usage_{};
		vk::DeviceMemory memory_{};

		uint32_t memory_index_{invalid_32_id};
		vk::MemoryRequirements memory_requirements_{};
		vk::MemoryPropertyFlags memory_property_flags_{};

		bool is_locked_{};
	};
}
