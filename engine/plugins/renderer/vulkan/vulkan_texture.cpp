#include "vulkan_texture.h"
#include "plugins/renderer/vulkan/renderer_vulkan.h"
#include "vulkan_types.h"

#include <memory>
#include <renderer/renderer_frontend.h>
namespace egkr
{
    static vk::Format channel_count_to_format(uint8_t channel_count, vk::Format default_format)
    {
	switch (channel_count)
	{
	case 1:
	    return vk::Format::eR8Unorm;
	case 2:
	    return vk::Format::eR8G8Unorm;
	case 3:
	    return vk::Format::eR8G8B8Unorm;
	case 4:
	    return vk::Format::eR8G8B8A8Unorm;
	default:
	    return default_format;
	}
    }

    vulkan_texture::shared_ptr vulkan_texture::create(const vulkan_context* context, uint32_t width_, uint32_t height_, const egkr::texture::properties& properties)
    {
	return std::make_shared<vulkan_texture>(context, width_, height_, properties);
    }

    vulkan_texture vulkan_texture::create_raw(const vulkan_context* context, uint32_t width, uint32_t height, const egkr::texture::properties& properties)
    {
	return vulkan_texture(context, width, height, properties);
    }

    void vulkan_texture::create_view(const properties& view_properties)
    {
	vk::ImageSubresourceRange subresource{};
	subresource.setBaseMipLevel(0).setBaseArrayLayer(0).setLevelCount(1).setAspectMask(view_properties.aspect_flags);

	subresource.setLayerCount(view_properties.texture_type == egkr::texture::type::cube ? 6 : 1);
	vk::ImageViewCreateInfo image_view_info{};
	image_view_info.setImage(image_).setFormat(view_properties.image_format).setSubresourceRange(subresource);

	switch (view_properties.texture_type)
	{
	case egkr::texture::type::texture_2d:
	    image_view_info.setViewType(vk::ImageViewType::e2D);
	    break;
	case egkr::texture::type::cube:
	    image_view_info.setViewType(vk::ImageViewType::eCube);
	    break;
	default:
	    break;
	}

	if (view_)
	{
	    context_->device.logical_device.destroyImageView(view_, context_->allocator);
	}
	view_ = context_->device.logical_device.createImageView(image_view_info, context_->allocator);
    }

    vulkan_texture::vulkan_texture(): texture({}) { }

    vulkan_texture::vulkan_texture(const vulkan_context* context, uint32_t width, uint32_t height, const egkr::texture::properties& texture_properties)
        : texture(texture_properties), context_{context}, width_{width}, height_{height}
    {
    }

    vulkan_texture::~vulkan_texture() { free(); }

    void vulkan_texture::populate_internal(const properties& texture_properties)
    {
	ZoneScoped;

	vk::ImageCreateInfo image_info{};

	image_info.setExtent({width_, height_, texture_properties.depth})
	    .setFormat(texture_properties.image_format)
	    .setMipLevels(texture_properties.mip_levels)
	    .setTiling(texture_properties.tiling)
	    .setInitialLayout(vk::ImageLayout::eUndefined)
	    .setUsage(texture_properties.usage)
	    .setSamples(texture_properties.samples)
	    .setSharingMode(texture_properties.sharing_mode);

	switch (texture_properties.texture_type)
	{
	case egkr::texture::type::texture_2d:
	case egkr::texture::type::cube:
	    image_info.setImageType(vk::ImageType::e2D);
	    break;
	default:
	    break;
	}

	if (texture_properties.texture_type == egkr::texture::type::cube)
	{
	    image_info.setFlags(vk::ImageCreateFlagBits::eCubeCompatible);
	}

	image_info.setArrayLayers(texture_properties.texture_type == egkr::texture::type::cube ? 6 : 1);

	if (image_)
	{
	    context_->device.logical_device.destroyImage(image_, context_->allocator);
	}
	image_ = context_->device.logical_device.createImage(image_info, context_->allocator);
	SET_DEBUG_NAME(context_, VkObjectType::VK_OBJECT_TYPE_IMAGE, (uint64_t)(VkImage)image_, properties_.name)
	if (image_ == vk::Image{})
	{
	    LOG_ERROR("Failed to create vulkan_image");
	}

	const auto memory_requirements = context_->device.logical_device.getImageMemoryRequirements(image_);
	auto memory_type = context_->device.find_memory_index(memory_requirements.memoryTypeBits, texture_properties.memory_properties);
	if (memory_type == invalid_32_id)
	{
	    LOG_ERROR("Required memory type not found");
	}

	vk::MemoryAllocateInfo memory_info{};
	memory_info.setAllocationSize(memory_requirements.size).setMemoryTypeIndex(memory_type);

	memory_ = context_->device.logical_device.allocateMemory(memory_info, context_->allocator);

	//TODO conigurable memory offset
	context_->device.logical_device.bindImageMemory(image_, memory_, 0);

	create_view(texture_properties);
    }

    void vulkan_texture::set_image(vk::Image image)
    {
	if (image_)
	{
	    context_->device.logical_device.destroyImage(image_);
	}
	image_ = image;
    }

    bool vulkan_texture::populate(const egkr::texture::properties& texture_properties, const uint8_t* texture_data)
    {
	ZoneScoped;

	if (texture_data)
	{
	    vk::DeviceSize image_size = texture_properties.width * texture_properties.height * texture_properties.channel_count * (texture_properties.texture_type == egkr::texture::type::cube ? 6 : 1);

	    auto image_format = vk::Format::eR8G8B8A8Unorm;

	    const vulkan_texture::properties image_properties{.image_format = image_format,
	        .tiling = vk::ImageTiling::eOptimal,
	        .usage = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment,
	        .memory_properties = vk::MemoryPropertyFlagBits::eDeviceLocal,
	        .aspect_flags = vk::ImageAspectFlagBits::eColor,
	        .texture_type = texture_properties.texture_type};
	    populate_internal(image_properties);

	    if (texture_data)
	    {
		write_data(0, image_size, texture_data);
	    }


	    increment_generation();

	    return true;
	}
	return false;
    }

    bool vulkan_texture::populate_writeable()
    {
	ZoneScoped;

	vk::ImageUsageFlags usage;
	vk::ImageAspectFlags aspect;
	vk::Format format;

	if ((uint32_t)(properties_.texture_flags & egkr::texture::flags::depth) != 0)
	{
	    usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
	    aspect = vk::ImageAspectFlagBits::eDepth;
	    format = context_->device.depth_format;
	}
	else
	{
	    usage = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment;
	    aspect = vk::ImageAspectFlagBits::eColor;
	    format = channel_count_to_format(properties_.channel_count, vk::Format::eR8G8B8A8Unorm);
	}

	{
	    const vulkan_texture::properties texture_properties{
	        .image_format = format,
	        .tiling = vk::ImageTiling::eOptimal,
	        .usage = usage,
	        .memory_properties = vk::MemoryPropertyFlagBits::eDeviceLocal,
	        .aspect_flags = aspect,
	    };
	    populate_internal(texture_properties);

	    increment_generation();

	    return true;
	}
    }

    void vulkan_texture::free()
    {
	ZoneScoped;

	destroy();
    }

    void vulkan_texture::set_view(vk::ImageView view)
    {
	ZoneScoped;

	const auto& logical_device = context_->device.logical_device;
	logical_device.waitIdle();
	if (view_)
	{
	    logical_device.destroyImageView(view_, context_->allocator);
	    view_ = VK_NULL_HANDLE;
	}
	view_ = view;
    }

    bool vulkan_texture::write_data(uint64_t offset, uint64_t size, const uint8_t* texture_data)
    {
	ZoneScoped;

	auto image_format = channel_count_to_format(properties_.channel_count, vk::Format::eR8G8B8A8Unorm);

	auto staging_buffer = renderbuffer::renderbuffer::create(renderbuffer::type::staging, size);
	staging_buffer->bind(0);

	staging_buffer->load_range(offset, size, texture_data);

	command_buffer single_use{};
	single_use.begin_single_use(context_, context_->device.graphics_command_pool);

	transition_layout(single_use, image_format, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
	copy_from_buffer(single_use, *(vk::Buffer*)staging_buffer->get_buffer());
	transition_layout(single_use, image_format, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

	single_use.end_single_use(context_, context_->device.graphics_command_pool, context_->device.graphics_queue);
	return true;
    }

    void vulkan_texture::read_data(uint64_t offset, uint64_t size, void* out_memory)
    {
	auto format = channel_count_to_format(properties_.channel_count, vk::Format::eR8G8B8A8Unorm);

	auto staging = renderbuffer::renderbuffer::create(renderbuffer::type::read, size);
	staging->bind(0);

	command_buffer single_use{};
	single_use.begin_single_use(context_, context_->device.graphics_command_pool);
	transition_layout(single_use, format, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferSrcOptimal);
	copy_to_buffer(single_use, *(vk::Buffer*)staging->get_buffer());
	transition_layout(single_use, format, vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

	single_use.end_single_use(context_, context_->device.graphics_command_pool, context_->device.graphics_queue);
	staging->read(offset, size, out_memory);
	staging->unbind();
    }

    void vulkan_texture::read_pixel(uint32_t x, uint32_t y, uint4* out_rgba)
    {
	auto format = channel_count_to_format(properties_.channel_count, vk::Format::eR8G8B8A8Unorm);

	auto staging = renderbuffer::renderbuffer::create(renderbuffer::type::read, sizeof(uint4));
	staging->bind(0);

	command_buffer single_use{};
	single_use.begin_single_use(context_, context_->device.graphics_command_pool);
	transition_layout(single_use, format, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferSrcOptimal);
	copy_pixel_to_buffer(single_use, *(vk::Buffer*)staging->get_buffer(), x, y);
	transition_layout(single_use, format, vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

	single_use.end_single_use(context_, context_->device.graphics_command_pool, context_->device.graphics_queue);
	staging->read(0, sizeof(uint4), out_rgba);
	staging->unbind();
    }

    bool vulkan_texture::resize(uint32_t width, uint32_t height)
    {
	ZoneScoped;

	destroy();

	const vulkan_texture::properties texture_properties{
	    .image_format = channel_count_to_format(properties_.channel_count, vk::Format::eR8G8B8A8Unorm),
	    .tiling = vk::ImageTiling::eOptimal,
	    .usage = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment,
	    .memory_properties = vk::MemoryPropertyFlagBits::eDeviceLocal,
	    .aspect_flags = vk::ImageAspectFlagBits::eColor,
	};
	width_ = width;
	height_ = height;

	populate_internal(texture_properties);

	create_view(texture_properties);

	increment_generation();
	return true;
    }

    void vulkan_texture::destroy()
    {
	ZoneScoped;

	if (context_)
	{
	    const auto& logical_device = context_->device.logical_device;
	    logical_device.waitIdle();
	    if (view_)
	    {
		logical_device.destroyImageView(view_, context_->allocator);
		view_ = VK_NULL_HANDLE;
	    }

	    if (memory_)
	    {
		logical_device.freeMemory(memory_, context_->allocator);
		memory_ = VK_NULL_HANDLE;
	    }
	    if ((int)(properties_.texture_flags & egkr::texture::flags::is_wrapped) == 0)
	    {

		if (image_)
		{
		    logical_device.destroyImage(image_, context_->allocator);
		    image_ = VK_NULL_HANDLE;
		}
	    }
	    context_ = nullptr;
	}
    }

    void vulkan_texture::transition_layout(command_buffer command_buffer, vk::Format /*format*/, vk::ImageLayout old_layout, vk::ImageLayout new_layout)
    {
	ZoneScoped;

	vk::ImageSubresourceRange subresource_range{};
	subresource_range.setBaseMipLevel(0).setLevelCount(1).setBaseArrayLayer(0).setAspectMask(vk::ImageAspectFlagBits::eColor);

	subresource_range.setLayerCount(properties_.texture_type == egkr::texture::type::cube ? 6 : 1);

	vk::ImageMemoryBarrier barrier{};
	barrier.setOldLayout(old_layout)
	    .setNewLayout(new_layout)
	    .setSrcQueueFamilyIndex(context_->device.graphics_queue_index)
	    .setDstQueueFamilyIndex(context_->device.graphics_queue_index)
	    .setImage(image_)
	    .setSubresourceRange(subresource_range);

	vk::PipelineStageFlags source_stage{};
	vk::PipelineStageFlags destination_stage{};

	if (old_layout == vk::ImageLayout::eUndefined && new_layout == vk::ImageLayout::eTransferDstOptimal)
	{
	    barrier.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
	    source_stage = vk::PipelineStageFlagBits::eTopOfPipe;
	    destination_stage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if (old_layout == vk::ImageLayout::eTransferDstOptimal && new_layout == vk::ImageLayout::eShaderReadOnlyOptimal)
	{
	    barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite).setDstAccessMask(vk::AccessFlagBits::eShaderRead);

	    source_stage = vk::PipelineStageFlagBits::eTransfer;
	    destination_stage = vk::PipelineStageFlagBits::eFragmentShader;
	}
	else if (old_layout == vk::ImageLayout::eTransferSrcOptimal && new_layout == vk::ImageLayout::eShaderReadOnlyOptimal)
	{
	    barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferRead).setDstAccessMask(vk::AccessFlagBits::eShaderRead);

	    source_stage = vk::PipelineStageFlagBits::eTransfer;
	    destination_stage = vk::PipelineStageFlagBits::eFragmentShader;
	}
	else if (old_layout == vk::ImageLayout::eUndefined && new_layout == vk::ImageLayout::eTransferSrcOptimal)
	{
	    barrier.setSrcAccessMask({}).setDstAccessMask(vk::AccessFlagBits::eTransferRead);

	    source_stage = vk::PipelineStageFlagBits::eTopOfPipe;
	    destination_stage = vk::PipelineStageFlagBits::eTransfer;
	}
	else
	{
	    LOG_FATAL("Unsupported transition");
	}

	command_buffer.get_handle().pipelineBarrier(source_stage, destination_stage, vk::DependencyFlags{}, nullptr, nullptr, barrier);
    }

    void vulkan_texture::copy_from_buffer(command_buffer command_buffer, vk::Buffer buffer)
    {
	ZoneScoped;

	vk::ImageSubresourceLayers subresource{};
	subresource.setAspectMask(vk::ImageAspectFlagBits::eColor).setMipLevel(0).setBaseArrayLayer(0);

	subresource.setLayerCount(properties_.texture_type == egkr::texture::type::cube ? 6 : 1);

	vk::BufferImageCopy image_copy{};
	image_copy.setBufferOffset(0).setBufferRowLength(0).setBufferImageHeight(0).setImageSubresource(subresource).setImageExtent({width_, height_, 1});

	command_buffer.get_handle().copyBufferToImage(buffer, image_, vk::ImageLayout::eTransferDstOptimal, image_copy);
    }

    void vulkan_texture::copy_to_buffer(command_buffer command_buffer, vk::Buffer buffer)
    {
	ZoneScoped;

	vk::ImageSubresourceLayers subresource{};
	subresource.setAspectMask(vk::ImageAspectFlagBits::eColor).setMipLevel(0).setBaseArrayLayer(0);

	subresource.setLayerCount(properties_.texture_type == egkr::texture::type::cube ? 6 : 1);

	vk::BufferImageCopy image_copy{};
	image_copy.setBufferOffset(0).setBufferRowLength(0).setBufferImageHeight(0).setImageSubresource(subresource).setImageExtent({width_, height_, 1});

	command_buffer.get_handle().copyImageToBuffer(image_, vk::ImageLayout::eTransferDstOptimal, buffer, image_copy);
    }
    void vulkan_texture::copy_pixel_to_buffer(command_buffer command_buffer, vk::Buffer buffer, uint32_t x, uint32_t y)
    {
	ZoneScoped;

	vk::ImageSubresourceLayers subresource{};
	subresource.setAspectMask(vk::ImageAspectFlagBits::eColor).setMipLevel(0).setBaseArrayLayer(0);

	subresource.setLayerCount(properties_.texture_type == egkr::texture::type::cube ? 6 : 1);

	vk::BufferImageCopy image_copy{};
	image_copy.setBufferOffset(0).setBufferRowLength(0).setBufferImageHeight(0).setImageSubresource(subresource).setImageExtent({1, 1, 1}).setImageOffset({(int32_t)x, (int32_t)y, 1});

	command_buffer.get_handle().copyImageToBuffer(image_, vk::ImageLayout::eTransferDstOptimal, buffer, image_copy);
    }

    static vk::SamplerAddressMode convert_repeat_type(std::string_view axis, egkr::texture_map::repeat repeat)
    {
	ZoneScoped;
	switch (repeat)
	{
	case egkr::texture_map::repeat::repeat:
	    return vk::SamplerAddressMode::eRepeat;
	case egkr::texture_map::repeat::mirrored_repeat:
	    return vk::SamplerAddressMode::eMirroredRepeat;
	case egkr::texture_map::repeat::clamp_to_edge:
	    return vk::SamplerAddressMode::eClampToEdge;
	case egkr::texture_map::repeat::clamp_to_border:
	    return vk::SamplerAddressMode::eClampToBorder;
	default:
	    LOG_WARN("Unknown address mode specified for {}. Defaulting to repeat.", axis.data());
	    return vk::SamplerAddressMode::eRepeat;
	}
    }

    static vk::Filter convert_filter_type(std::string_view op, egkr::texture_map::filter filter)
    {
	ZoneScoped;

	switch (filter)
	{
	case egkr::texture_map::filter::nearest:
	    return vk::Filter::eNearest;
	case egkr::texture_map::filter::linear:
	    return vk::Filter::eLinear;
	default:
	    LOG_WARN("Unknown filter tyoe for {}. Defaulting to nearest", op.data());
	    return vk::Filter::eNearest;
	}
    }

    vulkan_texture_map::vulkan_texture_map(const vulkan_context* context, const egkr::texture_map::properties& map_properties): egkr::texture_map(map_properties), context_{context} { }

    void vulkan_texture_map::acquire()
    {
	vk::SamplerCreateInfo create_info{};
	create_info.setMinFilter(convert_filter_type("min", minify))
	    .setMagFilter(convert_filter_type("mag", magnify))
	    .setAddressModeU(convert_repeat_type("u", repeat_u))
	    .setAddressModeV(convert_repeat_type("v", repeat_u))
	    .setAddressModeW(convert_repeat_type("w", repeat_u))
	    .setAnisotropyEnable(true)
	    .setMaxAnisotropy(16)
	    .setBorderColor(vk::BorderColor::eFloatOpaqueBlack)
	    .setUnnormalizedCoordinates(false)
	    .setCompareEnable(false)
	    .setMipmapMode(vk::SamplerMipmapMode::eLinear);

	sampler = context_->device.logical_device.createSampler(create_info, context_->allocator);
    }

    void vulkan_texture_map::release()
    {
	if (context_)
	{
	    context_->device.logical_device.waitIdle();
	    if (map_texture)
	    {
		map_texture->destroy();
	    }

	    if (sampler)
	    {
		context_->device.logical_device.destroySampler(sampler, context_->allocator);
		sampler = VK_NULL_HANDLE;
	    }

	    context_ = nullptr;
	}
    }

    vulkan_texture_map::~vulkan_texture_map() { release(); }
}
