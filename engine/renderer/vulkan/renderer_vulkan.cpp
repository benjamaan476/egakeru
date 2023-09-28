#include "renderer_vulkan.h"
#include "vulkan_types.h"

#include "platform/windows/platform_windows.h"
#include "swapchain.h"
#include "pipeline.h"

namespace egkr
{
	VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
		const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
		void* /*userData*/)
	{
		switch (messageSeverity)
		{
		case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			LOG_INFO("{0}", callbackData->pMessage);
			break;
		case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			LOG_WARN("{0}", callbackData->pMessage);
			break;
		case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			LOG_ERROR("{0}", callbackData->pMessage);
			break;
		case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			LOG_TRACE("{0}", callbackData->pMessage);
			break;
		default:
			//	LOG_ERROR("{0}: {1}", "Debug layer", callbackData->pMessage);
		}

		return (VkBool32)true;

	}

	int32_t vulkan_device::find_memory_index(uint32_t type_filter, vk::MemoryPropertyFlags property_flags) const
	{
		const auto device_properties = physical_device.getMemoryProperties();

		for (auto i{ 0U }; i < device_properties.memoryTypeCount; ++i)
		{
			if (type_filter & (1 << i) && (device_properties.memoryTypes[i].propertyFlags & property_flags) == property_flags)
			{
				return i;
			}
		}

		LOG_WARN("Failed to find suitable memory type");
		return -1;
	}

	void renderer_vulkan::create_command_buffers()
	{
		if (context_.graphics_command_buffers.empty())
		{
			context_.graphics_command_buffers.resize(context_.swapchain->get_image_count());
		}

		for (auto& buffer : context_.graphics_command_buffers)
		{
			if (buffer.get_handle() != VK_NULL_HANDLE)
			{
				buffer.free(&context_, context_.device.graphics_command_pool);
			}

			buffer.allocate(&context_, context_.device.graphics_command_pool, true);
		}
	}

	renderer_backend::unique_ptr renderer_vulkan::create(const platform::shared_ptr& platform)
	{
		return std::make_unique<renderer_vulkan>(platform);
	}

	renderer_vulkan::renderer_vulkan(platform::shared_ptr platform)
		:platform_{std::move(platform)}
	{
		const auto size = platform_->get_framebuffer_size();
		context_.framebuffer_width = size.x;
		context_.framebuffer_height = size.y;
	}

	renderer_vulkan::~renderer_vulkan()
	{
		shutdown();
	}

	bool renderer_vulkan::init()
	{
		if (!init_instance())
		{
            LOG_FATAL("Failed to create vulkan instance");
			return false;
		}

		if (enable_validation_layers_ && !create_debug_messenger())
		{
            LOG_FATAL("Failed to create debug messenger");
		}

		context_.surface = create_surface();
		if (!pick_physical_device())
		{
			LOG_FATAL("Failed to find suitable physical device");
			return false;
		}

		if (!create_logical_device())
		{
			LOG_FATAL("Failed to create logical device");
			return false;
		}

		context_.swapchain = swapchain::create(&context_);

		renderpass_properties main_renderpass_properties{};
		main_renderpass_properties.clear_colour = { 0.F, 0.F, 0.2F, 1.F };
		main_renderpass_properties.render_extent = { 0, 0, context_.framebuffer_width, context_.framebuffer_height };
		main_renderpass_properties.depth = 1.F;
		main_renderpass_properties.stencil = 0;

		context_.main_renderpass = renderpass::create(&context_, main_renderpass_properties);

		context_.swapchain->regenerate_framebuffers(context_.main_renderpass);

		create_command_buffers();

		context_.image_available_semaphore.resize(context_.swapchain->get_max_frames_in_flight());
		context_.queue_complete_semaphore.resize(context_.swapchain->get_max_frames_in_flight());

		context_.in_flight_fences.resize(context_.swapchain->get_image_count());
		context_.images_in_flight.resize(context_.swapchain->get_image_count());

		for (auto i{ 0U }; i < context_.swapchain->get_max_frames_in_flight(); ++i)
		{
			context_.image_available_semaphore[i] = context_.device.logical_device.createSemaphore({}, context_.allocator);
			context_.queue_complete_semaphore[i] = context_.device.logical_device.createSemaphore({}, context_.allocator);

			context_.in_flight_fences[i] = fence::create(&context_, true);
		}

		create_object_shader();

		return true;
	}

	void renderer_vulkan::shutdown()
	{
		if (context_.instance)
		{
			context_.device.logical_device.waitIdle();

			context_.object_shader_->destroy();
			for (auto i{ 0U }; i < context_.swapchain->get_max_frames_in_flight(); ++i)
			{
				context_.device.logical_device.destroySemaphore(context_.queue_complete_semaphore[i], context_.allocator);
				context_.device.logical_device.destroySemaphore(context_.image_available_semaphore[i], context_.allocator);
				context_.in_flight_fences[i]->destroy();
			}
			for (auto& buffer : context_.graphics_command_buffers)
			{
				if (buffer.get_handle())
				{
					buffer.free(&context_, context_.device.graphics_command_pool);
				}
			}
			context_.graphics_command_buffers.clear();

			context_.device.logical_device.destroyCommandPool(context_.device.graphics_command_pool);

			context_.main_renderpass.reset();
			context_.swapchain.reset();

			context_.instance.destroySurfaceKHR(context_.surface);
			if (enable_validation_layers_)
			{
				//DestroyDebugUtilsMessengerEXT(context_.instance, context_.debug, nullptr);
			}

			auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context_.instance, "vkDestroyDebugUtilsMessengerEXT");

			if (func != nullptr)
			{
				func(context_.instance, context_.debug, (VkAllocationCallbacks*)context_.allocator);
			}

			context_.device.logical_device.destroy();
			context_.instance.destroy();
			context_.instance = VK_NULL_HANDLE;

			platform_.reset();
		}
	}
	void renderer_vulkan::resize(uint32_t width, uint32_t height)
	{

		context_.framebuffer_width = width;
		context_.framebuffer_height = height;

		++context_.size_generation_;
	}

	bool renderer_vulkan::begin_frame(std::chrono::milliseconds /*delta_time*/)
	{
		if (context_.recreating_swapchain)
		{
			context_.device.logical_device.waitIdle();
			return false;
		}

		if (context_.size_generation_ != context_.last_size_generation_)
		{
			context_.device.logical_device.waitIdle();
			recreate_swapchain();
			return false;
		}

		context_.in_flight_fences[context_.current_frame]->wait(std::numeric_limits<uint64_t>::max());
		context_.image_index = context_.swapchain->acquire_next_image_index(context_.image_available_semaphore[context_.current_frame], VK_NULL_HANDLE);

		auto& command_buffer = context_.graphics_command_buffers[context_.image_index];
		command_buffer.reset();
		command_buffer.begin(false, false, false);

		vk::Viewport viewport{};
		viewport
			.setX(0)
			.setY(0)
			.setWidth(context_.framebuffer_width)
			.setHeight(context_.framebuffer_height)
			.setMinDepth(0.F)
			.setMaxDepth(1.F);

		vk::Rect2D scissor{};
		scissor
			.setOffset({ 0, 0 })
			.setExtent({ context_.framebuffer_width, context_.framebuffer_height });

		command_buffer.get_handle().setViewport(0, viewport);
		command_buffer.get_handle().setScissor(0, scissor);

		context_.main_renderpass->begin(command_buffer, context_.swapchain->get_framebuffer(context_.image_index)->get_handle());

		++frame_number_;
		return true;
	}

	void renderer_vulkan::end_frame()
	{ 
		auto& command_buffer = context_.graphics_command_buffers[context_.image_index];

		context_.main_renderpass->end(command_buffer);
		command_buffer.end();

		if (context_.images_in_flight[context_.image_index] != VK_NULL_HANDLE)
		{
			context_.images_in_flight[context_.image_index]->wait(std::numeric_limits<uint64_t>::max());
		}
			context_.images_in_flight[context_.image_index] = context_.in_flight_fences[context_.current_frame];

		context_.in_flight_fences[context_.current_frame]->reset();

		const vk::PipelineStageFlags stage_mask = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
		vk::SubmitInfo submit_info{};
		submit_info
			.setCommandBuffers(command_buffer.get_handle())
			.setSignalSemaphores(context_.queue_complete_semaphore[context_.current_frame])
			.setWaitSemaphores(context_.image_available_semaphore[context_.current_frame])
			.setWaitDstStageMask(stage_mask);

		context_.device.graphics_queue.submit(submit_info, context_.in_flight_fences[context_.current_frame]->get_handle());
		command_buffer.update_submitted();

		context_.swapchain->present(context_.device.graphics_queue, context_.device.present_queue, context_.queue_complete_semaphore[context_.current_frame], context_.image_index);
	}

	bool renderer_vulkan::init_instance()
	{
		vk::ApplicationInfo application_info{};
		application_info
			.setPApplicationName("engine")
			.setApplicationVersion(VK_MAKE_API_VERSION(0, 1, 0, 0))
			.setPEngineName("egakeru")
			.setEngineVersion(VK_MAKE_API_VERSION(0, 1, 0, 0))
			.setApiVersion(VK_API_VERSION_1_3);

		vk::InstanceCreateInfo instance_info{};
		instance_info.setPApplicationInfo(&application_info);

		auto extensions = platform_->get_required_extensions();

		if (enable_validation_layers_)
		{
			LOG_INFO("Adding debug validation");
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		instance_info.setEnabledExtensionCount(extensions.size());
		instance_info.ppEnabledExtensionNames = extensions.data();
		instance_info.setEnabledLayerCount(0);

		const auto layers = vk::enumerateInstanceLayerProperties();
		auto valid = std::ranges::all_of(validation_layers_,
				[&layers](const char* layer)
				{
				 auto layerFound = false;
				 for (const auto& layerProperty : layers)
				 {
					 if (strcmp(layer, layerProperty.layerName.data()) == 0)
					 {
						 layerFound = true;
						 break;
					 }
				 }

				 return layerFound;
				});

		if (enable_validation_layers_ && !valid)
		{
			LOG_FATAL("Invalid layer extension selected");
			return false;
		}

		if (enable_validation_layers_)
		{
			instance_info.setEnabledLayerCount((uint32_t)validation_layers_.size());
			instance_info.ppEnabledLayerNames = validation_layers_.data();
		}

		context_.instance = vk::createInstance(instance_info, context_.allocator);
		if (context_.instance == vk::Instance{})
		{
			LOG_FATAL("Failed to create vulkan instance");
			return false;
		}

		LOG_INFO("Vulkan instance initialised");
		return true;
	}

	bool renderer_vulkan::create_debug_messenger()
	{
		if (!enable_validation_layers_)
		{
			return false;
		}

		VkDebugUtilsMessengerCreateInfoEXT create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		create_info.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;

		create_info.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;

		create_info.pUserData = nullptr;
		create_info.pfnUserCallback = debugCallback;

		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context_.instance, "vkCreateDebugUtilsMessengerEXT");

		if (func != nullptr)
		{
			func(context_.instance, &create_info, (VkAllocationCallbacks*)context_.allocator, &context_.debug);
		}

		return true;
	}

	vk::SurfaceKHR renderer_vulkan::create_surface()
	{
		return platform_->create_surface(context_.instance);
	}

	bool renderer_vulkan::pick_physical_device()
	{
		const auto devices = context_.instance.enumeratePhysicalDevices();

		if (devices.empty())
		{
			LOG_FATAL("No supported vulkan devices found");
			return false;
		}

		for (const auto& device : devices)
		{
			if (is_device_suitable(device))
			{
				context_.device.physical_device = device;
				break;
			}
		}

		auto supported = check_device_extension_support(context_.device.physical_device);
		if (!supported)
		{
			LOG_ERROR("Selected device does not support swapchain");
			return false;
		}

		return true;
	}
	bool renderer_vulkan::is_device_suitable(const vk::PhysicalDevice& device)
	{
		auto queueIndices = find_queue_families(context_, device);

		auto properties = device.getProperties();
		auto features = device.getFeatures();

		auto swapChainSupport = query_swapchain_support(context_, device);
		auto swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.present_modes.empty();

		return (properties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu || properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) && (bool)features.geometryShader && queueIndices.is_complete() && swapChainAdequate;
	}

	bool renderer_vulkan::check_device_extension_support(const vk::PhysicalDevice& physical_device)
	{
		auto extension_properties = physical_device.enumerateDeviceExtensionProperties();

		std::set<std::string> required_extensions(device_extensions_.begin(), device_extensions_.end());

		for (const auto& extension : extension_properties)
		{
			required_extensions.erase(extension.extensionName);
		}

		return required_extensions.empty();
	}

	bool renderer_vulkan::create_logical_device()
	{
		auto queue_indices = find_queue_families(context_, context_.device.physical_device);

		egkr::vector<vk::DeviceQueueCreateInfo> device_queue_create_infos{};

		std::set<uint32_t> unique_queue_familes{};
		if (queue_indices.graphics_family.has_value() && queue_indices.present_family.has_value())
		{
			 unique_queue_familes = { queue_indices.graphics_family.value(), queue_indices.present_family.value() };
		}
		else
		{
			LOG_ERROR("Could not find family indices");
			return false;
		}
		const auto queue_priority = 1.F;
		for (auto queue_family : unique_queue_familes)
		{
			vk::DeviceQueueCreateInfo device_queue_create_info{};
			device_queue_create_info
				.setQueueFamilyIndex(queue_family)
				.setQueueCount(1)
				.setQueuePriorities(queue_priority);

			device_queue_create_infos.push_back(device_queue_create_info);
		}

		const auto physical_device_features = context_.device.physical_device.getFeatures();

		vk::DeviceCreateInfo device_create_info{};
		device_create_info
			.setQueueCreateInfos(device_queue_create_infos)
			.setPEnabledFeatures(&physical_device_features)
			.setEnabledExtensionCount((uint32_t)device_extensions_.size());

		device_create_info.ppEnabledExtensionNames = device_extensions_.data();

		if (enable_validation_layers_)
		{
			device_create_info.setEnabledLayerCount((uint32_t)validation_layers_.size());
			device_create_info.ppEnabledLayerNames = validation_layers_.data();
		}
		else
		{
			device_create_info.setEnabledLayerCount(0);
		}

		auto& device = context_.device;

		device.logical_device = device.physical_device.createDevice(device_create_info, context_.allocator);
		if (device.logical_device == vk::Device{})
		{
			LOG_ERROR("Could not create logical device");
			return false;
		}

		device.graphics_queue_index = queue_indices.graphics_family.value();
		device.graphics_queue = device.logical_device.getQueue(device.graphics_queue_index, 0);

		device.present_queue_index = queue_indices.present_family.value();
		device.present_queue = device.logical_device.getQueue(device.present_queue_index, 0);

		vk::CommandPoolCreateInfo create_pool{};
		create_pool
			.setQueueFamilyIndex(device.graphics_queue_index)
			.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

		device.graphics_command_pool = device.logical_device.createCommandPool(create_pool, context_.allocator);

		return true;
	}

	bool renderer_vulkan::recreate_swapchain()
	{
		if (context_.recreating_swapchain)
		{
			return false;
		}

		if (context_.framebuffer_height == 0 || context_.framebuffer_width == 0)
		{
			LOG_INFO("Framebuffer called with zero dimension");
			return false;
		}

		context_.device.logical_device.waitIdle();

		for (auto& image_in_flight : context_.images_in_flight)
		{
			image_in_flight = VK_NULL_HANDLE;
		}

		//query_swapchain_support(*context_, context_->device.physical_device);
		//detect_depth_format(context_->device);


		context_.last_size_generation_ = context_.size_generation_;
		for (auto i{ 0U }; i < context_.swapchain->get_image_count(); ++i)
		{
			context_.graphics_command_buffers[i].free(&context_, context_.device.graphics_command_pool);
			context_.swapchain->get_framebuffer(i)->destroy();
		}

		context_.swapchain->recreate();

		context_.main_renderpass->set_extent({ 0, 0, context_.framebuffer_width, context_.framebuffer_height });

		context_.swapchain->regenerate_framebuffers(context_.main_renderpass);

		create_command_buffers();
		context_.recreating_swapchain = false;
		return true;
	}

	void renderer_vulkan::create_object_shader()
	{
		context_.object_shader_ = shader::create(&context_);

		//TODO Descriptors

		vk::Viewport viewport(0, context_.framebuffer_height, context_.framebuffer_width, -(float)context_.framebuffer_height, 0.F, 1.F);
		vk::Rect2D scissor({ 0U, 0U }, { context_.framebuffer_width, context_.framebuffer_height});

		pipeline_properties object_pipeline_properties{};
		object_pipeline_properties.is_wireframe = false;
		object_pipeline_properties.scissor = scissor;
		object_pipeline_properties.viewport = viewport;
		object_pipeline_properties.shader_stage_info = context_.object_shader_->get_shader_stages();
		object_pipeline_properties.vertex_attributes = vertex_3d::get_attribute_description();
		object_pipeline_properties.renderpass = context_.main_renderpass;

		context_.object_shader_->set_pipeline(pipeline::create(&context_, object_pipeline_properties));

	}
}

