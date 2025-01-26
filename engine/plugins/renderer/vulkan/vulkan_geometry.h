#pragma once

#include "resources/geometry.h"
#include "vulkan_renderbuffer.h"

namespace egkr
{
	class vulkan_geometry : public geometry
	{
	public:
		static shared_ptr create(const vulkan_context* context, const properties& properties);

		vulkan_geometry(const vulkan_context* context, const properties& properties);
		~vulkan_geometry() override;

		bool populate(const properties& properties) override;
		bool upload() override;
		void draw() override;
		void update_vertices(uint32_t offset, uint32_t vertex_count, void* vertices) override;
		void free() override;
	private:
		const vulkan_context* context_{};

	};

}
