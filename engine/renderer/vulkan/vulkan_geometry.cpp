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

		const auto vertex_buffer_size = properties.vertex_size * properties.vertex_count;

		vertex_count_ = properties.vertex_count;
		vertex_size_ = properties.vertex_size;

		vertex_buffer_ = renderbuffer::renderbuffer::create(backend_, renderbuffer::type::vertex, vertex_buffer_size);
		vertex_buffer_->bind(0);

		vertex_buffer_->load_range(0, vertex_buffer_size, properties.vertices);

		index_count_ = properties.indices.size();
		if (index_count_)
		{
			const auto index_buffer_size = sizeof(uint32_t) * properties.indices.size();
			index_buffer_ = renderbuffer::renderbuffer::create(backend_, renderbuffer::type::index, index_buffer_size);
			index_buffer_->bind(0);
			index_buffer_->load_range(0, index_buffer_size, properties.indices.data());
		}
		return true;
	}

	void vulkan_geometry::draw()
	{
		bool includes_index_data = index_count_ > 0;

		vertex_buffer_->draw(0, vertex_count_, includes_index_data);

		if (includes_index_data)
		{
			index_buffer_->draw(0, index_count_, !includes_index_data);
		}
	}

	void vulkan_geometry::update_vertices(uint32_t offset, uint32_t vertex_count, void* vertices)
	{
		if (vertex_count > vertex_count_)
		{
			LOG_ERROR("Cannot currently add vertices to a geometry. Can only edit existing vertices.");
			return;
		}

		uint32_t total_size = vertex_count * vertex_size_;
		vertex_buffer_->load_range(offset, total_size, vertices);
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
				vertex_buffer_.reset();
			}

			if (index_buffer_)
			{
				index_buffer_.reset();
			}
		}
	}
}