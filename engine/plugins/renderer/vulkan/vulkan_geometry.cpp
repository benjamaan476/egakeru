#include "vulkan_geometry.h"
#include "renderer/renderer_types.h"
#include "vulkan_types.h"
#include "systems/material_system.h"
#include <vulkan/vulkan_core.h>

namespace egkr
{
    geometry::shared_ptr vulkan_geometry::create(const vulkan_context* context, const properties& properties)
    {
	auto geom = std::make_shared<vulkan_geometry>(context, properties);

	if (properties.vertex_count)
	{
	    geom->populate(properties);
	    geom->upload();
	}
	else
	{
	    LOG_WARN("Geometry has no vertex data. Not populating");
	}
	return geom;
    }

    vulkan_geometry::vulkan_geometry(const vulkan_context* context, const properties& geometry_properties): geometry(geometry_properties), context_{context} { }

    vulkan_geometry::~vulkan_geometry()
    {
	free();
    }

    bool vulkan_geometry::populate(const properties& geometry_properties)
    {
	ZoneScoped;

	const auto vertex_buffer_size = geometry_properties.vertex_size * geometry_properties.vertex_count;

	vertex_count_ = geometry_properties.vertex_count;
	vertex_size_ = geometry_properties.vertex_size;
	vertices_ = malloc(vertex_buffer_size);
	std::memcpy(vertices_, geometry_properties.vertices, vertex_buffer_size);

	vertex_buffer_ = renderbuffer::renderbuffer::create(renderbuffer::type::vertex, vertex_buffer_size);
	vertex_buffer_->bind(0);


	index_count_ = (uint32_t)geometry_properties.indices.size();
	if (index_count_)
	{
	    const auto index_buffer_size = sizeof(uint32_t) * geometry_properties.indices.size();
	    indices_ = geometry_properties.indices;
	    index_buffer_ = renderbuffer::renderbuffer::create(renderbuffer::type::index, index_buffer_size);
	    index_buffer_->bind(0);
	}
	return true;
    }

    bool vulkan_geometry::upload()
    {
	const auto vertex_buffer_size = vertex_size_ * vertex_count_;
	vertex_buffer_->load_range(0, vertex_buffer_size, vertices_);

	if (index_count_)
	{
	    const auto index_buffer_size = sizeof(uint32_t) * indices_.size();
	    index_buffer_->load_range(0, index_buffer_size, indices_.data());
	}
	return true;
    }

    void vulkan_geometry::draw()
    {
	if (!vertex_buffer_)
	{
	    LOG_WARN("Tried to render geometry without valid vertex buffer");
	    return;
	}
	const bool includes_index_data = index_count_ > 0;

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

	const uint32_t total_size = vertex_count * vertex_size_;
	vertex_buffer_->load_range(offset, total_size, vertices);
    }

    void vulkan_geometry::free()
    {
	ZoneScoped;

	if (context_)
	{
	    context_->device.logical_device.waitIdle();
	    if (material_)
	    {
		material_system::release(material_);
	    }
	    if (vertex_buffer_)
	    {
		vertex_buffer_.reset();
	    }

	    if (index_buffer_)
	    {
		index_buffer_.reset();
	    }
	    context_ = VK_NULL_HANDLE;
	}

	if (vertices_)
	{
	    ::free(vertices_);
	    vertices_ = nullptr;
	}
    }
}
