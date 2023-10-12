#include "renderer_vulkan.h"
#include "vulkan_types.h"

#include "platform/windows/platform_windows.h"
#include "swapchain.h"
#include "pipeline.h"

#include "vulkan_material.h"
#include "vulkan_geometry.h"
#include "vulkan_shader.h"
#include "resources/shader.h"

#include "systems/texture_system.h"
#include "systems/resource_system.h"

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

	bool vulkan_device::physical_device_meets_requirements(
		vk::PhysicalDevice device,
		vk::SurfaceKHR surface,
		const vk::PhysicalDeviceProperties& properties, 
		const vk::PhysicalDeviceFeatures features, 
		const physical_device_requirements& requirements,
		physical_device_queue_family_info& family_info,
		swapchain_support_details& swapchain_support)
	{
		if (requirements.discrete_gpu)
		{
			if (properties.deviceType != vk::PhysicalDeviceType::eDiscreteGpu)
			{
				LOG_INFO("Device is not a discrete GPU when one is required, skipping");
				return false;
			}
	}

		auto queue_families = device.getQueueFamilyProperties();

		LOG_INFO("Graphics | Present | Compute | Transfer | Name");
		uint8_t min_transfer_score{ 255 };

		for (auto i{ 0U }; i < queue_families.size(); ++i)
		{
			auto& queue_family = queue_families[i];

			uint8_t current_transfer_score{};

			if (queue_family.queueFlags & vk::QueueFlagBits::eGraphics)
			{
				family_info.graphics_index = i;
				++current_transfer_score;
			}

			if (queue_family.queueFlags & vk::QueueFlagBits::eCompute)
			{
				family_info.compute_index = i;
				++current_transfer_score;
			}

			if (queue_family.queueFlags & vk::QueueFlagBits::eTransfer)
			{
				if (current_transfer_score <= min_transfer_score)
				{
					min_transfer_score = current_transfer_score;
					family_info.transfer_index = i;
				}
			}

			auto supports_present = device.getSurfaceSupportKHR(i, surface);
			if (supports_present)
			{
				family_info.present_index = i;
			}
		}

		LOG_INFO("       {:d}|       {:d}|       {:d}|       {:d}| {}", 
				 family_info.graphics_index != -1,
				 family_info.present_index != -1, 
				 family_info.compute_index != -1, 
				 family_info.transfer_index != -1, 
				 properties.deviceName.data());

		if (
			(!requirements.graphics || (requirements.graphics && family_info.graphics_index != -1)) &&
		(!requirements.present || (requirements.present && family_info.present_index != -1)) &&
			(!requirements.compute || (requirements.compute && family_info.compute_index != -1)) &&
			(!requirements.transfer || (requirements.transfer && family_info.transfer_index != -1)))
		{
			LOG_INFO("Device meets requirements");

			swapchain_support = query_swapchain_support(surface, device);

			if (swapchain_support.formats.size() < 1 || swapchain_support.present_modes.size() < 1)
			{
				LOG_INFO("Required swapchain support not present, skipping device");
				return false;
			}

			if (!requirements.extension_names.empty())
			{
				auto available_extensions = device.enumerateDeviceExtensionProperties();
				for (const auto& extension : available_extensions)
				{
					bool found{};
					for (auto i{ 0U }; i < available_extensions.size(); ++i)
					{
						if (strcmp(requirements.extension_names[i], extension.extensionName) == 0)
						{
							found = true;
							break;
						}
					}

					if (!found)
					{
						LOG_INFO("Required extension not found, skipping");
						return false;
					}
				}
			}

		}

		if (requirements.sampler_anisotropy && !features.samplerAnisotropy)
		{
			LOG_INFO("Device does not support samplerAnisotropy, skipping");
			return false;
		}

		return true;
	}

	bool vulkan_device::select_physical_device(vulkan_context* context)
	{
		auto physical_devices = context->instance.enumeratePhysicalDevices();

		for (auto& physical_device : physical_devices)
		{
			const auto properties = physical_device.getProperties();
			const auto features = physical_device.getFeatures();
			const auto memory = physical_device.getMemoryProperties();

			physical_device_requirements requirements{};
			requirements.graphics = true;
			requirements.present = true;
			requirements.transfer = true;
			requirements.sampler_anisotropy = true;

			requirements.discrete_gpu = false;

			physical_device_queue_family_info queue_info{};

			auto result = physical_device_meets_requirements(physical_device, context->surface, properties, features, requirements, queue_info, context->device.swapchain_supprt);

			if (result)
			{
				LOG_INFO("Selected device: {}", properties.deviceName.data());
				switch (properties.deviceType)
				{
					using enum vk::PhysicalDeviceType;
				case eOther:
					LOG_INFO("Device type is unknown");
					break;
				case eIntegratedGpu:
					LOG_INFO("Device type is integrated");
					break;
				case eDiscreteGpu:
					LOG_INFO("Device type is discrete");
					break;
				default:
					break;
				}

				LOG_INFO("GPU Driver version: {}.{}.{}", VK_VERSION_MAJOR(properties.driverVersion), VK_VERSION_MINOR(properties.driverVersion), VK_VERSION_PATCH(properties.driverVersion));
				LOG_INFO("Vulkan API version: {}.{}.{}", VK_VERSION_MAJOR(properties.apiVersion), VK_VERSION_MINOR(properties.apiVersion), VK_VERSION_PATCH(properties.apiVersion));

				context->device.physical_device = physical_device;
				context->device.graphics_queue_index = queue_info.graphics_index;
				context->device.present_queue_index = queue_info.present_index;
				context->device.transfer_queue_index = queue_info.transfer_index;

				context->device.properties = properties;
				context->device.features = features;
				context->device.memory = memory;
				break;
			}
		}

		if (!context->device.physical_device)
		{
			LOG_ERROR("No physical devices found that support the requirements");
		}
		return true;
	}

	bool vulkan_device::create(vulkan_context* context)
	{
		if (!select_physical_device(context))
		{
			return false;
		}

		LOG_INFO("Creating logical device");
		const bool present_shares_graphics_queue = graphics_queue_index == present_queue_index;
		const bool transfer_shares_graphics_queue = graphics_queue_index == transfer_queue_index;

		uint32_t index_count{ 1 };
		if (!present_shares_graphics_queue)
		{
			++index_count;
		}

		if (!transfer_shares_graphics_queue)
		{
			++index_count;
		}

		std::array<uint32_t, 32> indices{};
		uint8_t index{};

		indices[index++] = graphics_queue_index;
		if (!present_shares_graphics_queue)
		{
			indices[index++] = present_queue_index;
		}

		if (!transfer_shares_graphics_queue)
		{
			indices[index++] = transfer_queue_index;
		}

		std::array<vk::DeviceQueueCreateInfo, 32> device_queue_create_infos{};

		for (auto i{ 0U }; i < index_count; ++i)
		{
			const float queue_priority{ 1.F };
			device_queue_create_infos[i]
				.setQueueFamilyIndex(indices[i])
				.setQueueCount(1)
				.setPQueuePriorities(&queue_priority);
		}

		vk::PhysicalDeviceFeatures device_features{};
		device_features.samplerAnisotropy = true;

		auto available_extensions = physical_device.enumerateDeviceExtensionProperties();
		bool portability_required{};
		for (const auto& extension : available_extensions)
		{
			if (strcmp(extension.extensionName, "VK_KHR_portability_subset") == 0)
			{
				LOG_INFO("Adding required extension 'VK_KHR_portability_subset'");
				portability_required = true;
				break;
			}
		}

		std::vector<const char*> extension_names{};
		extension_names.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		if (portability_required)
		{
			extension_names.push_back("VK_KHR_portability_subset");
		}

		vk::DeviceCreateInfo device_create_info{};
		device_create_info
			.setQueueCreateInfos(device_queue_create_infos)
			.setQueueCreateInfoCount(index_count)
			.setPEnabledFeatures(&device_features)
			.setPEnabledExtensionNames(extension_names);

		logical_device = physical_device.createDevice(device_create_info, context->allocator);
		
		graphics_queue = logical_device.getQueue(graphics_queue_index, 0);
		present_queue = logical_device.getQueue(present_queue_index, 0);
		transfer_queue = logical_device.getQueue(transfer_queue_index, 0);

		vk::CommandPoolCreateInfo pool_create_info{};
		pool_create_info
			.setQueueFamilyIndex(graphics_queue_index)
			.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

		graphics_command_pool = logical_device.createCommandPool(pool_create_info, context->allocator);
		return true;
	}

	void renderer_vulkan::create_command_buffers()
	{
		if (context_.graphics_command_buffers.empty())
		{
			context_.graphics_command_buffers.resize(context_.swapchain->get_image_count());
		}

		for (auto& buffer : context_.graphics_command_buffers)
		{
			if (buffer.get_handle())
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

		context_.device.create(&context_);

		context_.swapchain = swapchain::create(&context_);

		renderpass_properties main_renderpass_properties{};
		main_renderpass_properties.clear_colour = { 0.F, 0.F, 0.2F, 1.F };
		main_renderpass_properties.render_extent = { 0, 0, context_.framebuffer_width, context_.framebuffer_height };
		main_renderpass_properties.depth = 1.F;
		main_renderpass_properties.stencil = 0;
		main_renderpass_properties.clear_flags = renderpass_clear_flags::colour | renderpass_clear_flags::depth | renderpass_clear_flags::stencil;
		main_renderpass_properties.has_previous_pass = false;
		main_renderpass_properties.has_next_pass = true;

		context_.world_renderpass = renderpass::create(&context_, main_renderpass_properties);

		renderpass_properties ui_renderpass_properties{};
		ui_renderpass_properties.clear_flags = renderpass_clear_flags::none;
		ui_renderpass_properties.render_extent = { 0, 0, context_.framebuffer_width, context_.framebuffer_height };
		ui_renderpass_properties.has_previous_pass = true;
		ui_renderpass_properties.has_next_pass = false;

		context_.ui_renderpass = renderpass::create(&context_, ui_renderpass_properties);

		context_.swapchain->regenerate_framebuffers();

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

		return true;
	}

	void renderer_vulkan::shutdown()
	{
		if (context_.instance)
		{
			context_.device.logical_device.waitIdle();

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

			context_.ui_renderpass.reset();
			context_.world_renderpass.reset();
			context_.world_framebuffers.clear();
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

		++context_.framebuffer_size_generation;
	}

	bool renderer_vulkan::begin_frame(double /*delta_time*/)
	{
		if (context_.recreating_swapchain)
		{
			context_.device.logical_device.waitIdle();
			return false;
		}

		if (context_.framebuffer_size_generation != context_.framebuffer_last_size_generation)
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
			.setY(context_.framebuffer_height)
			.setWidth(context_.framebuffer_width)
			.setHeight(-(float)context_.framebuffer_height)
			.setMinDepth(0.F)
			.setMaxDepth(1.F);

		vk::Rect2D scissor{};
		scissor
			.setOffset({ 0, 0 })
			.setExtent({ context_.framebuffer_width, context_.framebuffer_height });

		command_buffer.get_handle().setViewport(0, viewport);
		command_buffer.get_handle().setScissor(0, scissor);



		++frame_number_;
		return true;
	}

	bool renderer_vulkan::begin_renderpass(builtin_renderpass renderpass)
	{
		auto& command_buffer = context_.graphics_command_buffers[context_.image_index];
		switch (renderpass)
		{
		case egkr::builtin_renderpass::world:
			context_.world_renderpass->begin(command_buffer, context_.world_framebuffers[context_.image_index]->get_handle());
			break;
		case egkr::builtin_renderpass::ui:
			context_.ui_renderpass->begin(command_buffer, context_.swapchain->get_framebuffer(context_.image_index)->get_handle());
			break;
		default:
			LOG_ERROR("Begin renderpass called with invalid id");
			return false;
		}

		return true;
	}

	bool renderer_vulkan::end_renderpass(builtin_renderpass renderpass)
	{
		auto& command_buffer = context_.graphics_command_buffers[context_.image_index];
		switch (renderpass)
		{
		case egkr::builtin_renderpass::world:
			context_.world_renderpass->end(command_buffer);
			break;
		case egkr::builtin_renderpass::ui:
			context_.ui_renderpass->end(command_buffer);
			break;
		default:
			LOG_ERROR("Begin renderpass called with invalid id");
			return false;
		}
		return true;
	}

	//void renderer_vulkan::update_world_state(const float4x4& projection, const float4x4& view, const float3& /*view_position*/, const float4& /*ambient_colour*/, int32_t /*mode*/)
	//{
	//	context_.material_shader->update_global_state({ projection, view });
	//}

	//void renderer_vulkan::update_ui_state(const float4x4& projection, const float4x4& view, const float3& /*view_position*/, const float4& /*ambient_colour*/, int32_t /*mode*/)
	//{
	//	context_.ui_shader->update_global_state({ projection, view });
	//}

	void renderer_vulkan::draw_geometry(const geometry_render_data& data)
	{
		const auto& geometry = data.geometry;
		const auto& state = (vulkan_geometry_state*)geometry->data;

		auto& command_buffer = context_.graphics_command_buffers[context_.image_index];
		vk::DeviceSize offset{ 0 };
		command_buffer.get_handle().bindVertexBuffers(0, state->vertex_buffer_->get_handle(), offset);

		command_buffer.get_handle().bindIndexBuffer(state->index_buffer_->get_handle(), offset, vk::IndexType::eUint32);

		if (state->index_count_)
		{
			command_buffer.get_handle().drawIndexed(state->index_count_, 1, 0, 0, 0);
		}
		else
		{
			command_buffer.get_handle().draw(state->vertex_count_, 0, 0, 0);
		}

	}

	void renderer_vulkan::end_frame()
	{ 
		auto& command_buffer = context_.graphics_command_buffers[context_.image_index];
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

		auto swapChainSupport = query_swapchain_support(context_.surface, device);
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


		context_.framebuffer_last_size_generation = context_.framebuffer_size_generation;
		for (auto i{ 0U }; i < context_.swapchain->get_image_count(); ++i)
		{
			context_.graphics_command_buffers[i].free(&context_, context_.device.graphics_command_pool);
			context_.swapchain->get_framebuffer(i)->destroy();
		}

		context_.swapchain->recreate();

		context_.world_renderpass->set_extent({ 0, 0, context_.framebuffer_width, context_.framebuffer_height });
		context_.ui_renderpass->set_extent({ 0, 0, context_.framebuffer_width, context_.framebuffer_height });

		context_.swapchain->regenerate_framebuffers();

		create_command_buffers();
		context_.recreating_swapchain = false;
		return true;
	}

	void renderer_vulkan::free_material(material* material)
	{
		delete (vulkan_material_state*)material->data;
	}


	bool renderer_vulkan::populate_texture(texture* texture, const texture_properties& properties, const uint8_t* data)
	{
		vulkan_texture_state state{};
		state.context = &context_;
		if (data)
		{
			vk::DeviceSize image_size = properties.width * properties.height * properties.channel_count;
			auto staging_buffer = buffer::create(&context_, image_size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, true);
			staging_buffer->load_data(0, image_size, 0, data);

			image_properties image_properties{};
			image_properties.tiling = vk::ImageTiling::eOptimal;
			image_properties.usage = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment;
			image_properties.memory_properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
			image_properties.image_format = vk::Format::eR8G8B8A8Srgb;
			image_properties.aspect_flags = vk::ImageAspectFlagBits::eColor;

			state.image = image::create(&context_, properties.width, properties.height, image_properties, true);

			command_buffer single_use{};
			single_use.begin_single_use(&context_, context_.device.graphics_command_pool);

			state.image->transition_layout(single_use, vk::Format::eR8G8B8Srgb, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
			state.image->copy_from_buffer(single_use, staging_buffer);
			state.image->transition_layout(single_use, vk::Format::eR8G8B8Srgb, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

			single_use.end_single_use(&context_, context_.device.graphics_command_pool, context_.device.graphics_queue);

			vk::SamplerCreateInfo sampler_info{};
			sampler_info
				.setMinFilter(vk::Filter::eLinear)
				.setMagFilter(vk::Filter::eLinear)
				.setAddressModeU(vk::SamplerAddressMode::eRepeat)
				.setAddressModeV(vk::SamplerAddressMode::eRepeat)
				.setAddressModeW(vk::SamplerAddressMode::eRepeat)
				.setAnisotropyEnable(true)
				.setMaxAnisotropy(16)
				.setBorderColor(vk::BorderColor::eFloatOpaqueBlack)
				.setUnnormalizedCoordinates(false)
				.setCompareEnable(false)
				.setMipmapMode(vk::SamplerMipmapMode::eLinear);

			state.sampler = context_.device.logical_device.createSampler(sampler_info, context_.allocator);
		}

		texture->data = new vulkan_texture_state();
		*(vulkan_texture_state*)texture->data = state;

		return true;
	}

	void renderer_vulkan::free_texture(texture* texture)
	{
		context_.device.logical_device.waitIdle();
		auto state = (vulkan_texture_state*)texture->data;
		if (state->sampler)
		{
			context_.device.logical_device.destroySampler(state->sampler, context_.allocator);
			state->sampler = VK_NULL_HANDLE;
		}
		if (state->image)
		{
			state->image.reset();
		}

		delete (vulkan_texture_state*)texture->data;
		texture->data = nullptr;
	}

	bool renderer_vulkan::populate_geometry(geometry* geometry, const geometry_properties& properties)
	{
		geometry->data = new vulkan_geometry_state();
		auto state = (vulkan_geometry_state*)geometry->data;

		const vk::MemoryPropertyFlags flags{vk::MemoryPropertyFlagBits::eDeviceLocal};
		const vk::BufferUsageFlags usage{vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc};
		const auto vertex_buffer_size = properties.vertex_size * properties.vertex_count;

		state->vertex_count_ = properties.vertex_count;
		state->vertex_buffer_ = buffer::create(&context_, vertex_buffer_size, usage, flags, true);

		const vk::BufferUsageFlags index_usage{vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc};
		const auto index_buffer_size = sizeof(uint32_t) * properties.indices.size();

		state->index_count_ = properties.indices.size();
		state->index_buffer_ = buffer::create(&context_, index_buffer_size, index_usage, flags, true);
		upload_data_range(&context_, context_.device.graphics_command_pool, VK_NULL_HANDLE, context_.device.graphics_queue, state->vertex_buffer_, 0, vertex_buffer_size, properties.vertices);
		upload_data_range(&context_, context_.device.graphics_command_pool, VK_NULL_HANDLE, context_.device.graphics_queue, state->index_buffer_, 0, index_buffer_size, properties.indices.data());

		return true;
	}

	void renderer_vulkan::free_geometry(geometry* geometry)
	{
		context_.device.logical_device.waitIdle();

		auto state = (vulkan_geometry_state*)geometry->data;
		if (state)
		{

			state->vertex_buffer_.reset();
			state->index_buffer_.reset();

			delete state;
			state = nullptr;
		}

	}

	bool renderer_vulkan::populate_shader(shader* shader, uint32_t renderpass_id, const egkr::vector<std::string>& stage_filenames, const egkr::vector<shader_stages>& shader_stages)
	{
		renderpass::shared_ptr renderpass{};
		if (renderpass_id == 1)
		{
			renderpass = context_.world_renderpass;
		}
		else
		{
			renderpass = context_.ui_renderpass;
		}

		egkr::vector<vk::ShaderStageFlagBits> stages{};
		for (const auto stage : shader_stages)
		{
			switch (stage)
			{
			case shader_stages::fragment:
				stages.push_back(vk::ShaderStageFlagBits::eFragment);
				break;
			case shader_stages::vertex:
				stages.push_back(vk::ShaderStageFlagBits::eVertex);
				break;
			case shader_stages::geometry:
				stages.push_back(vk::ShaderStageFlagBits::eGeometry);
				break;
			case shader_stages::compute:
				stages.push_back(vk::ShaderStageFlagBits::eCompute);
				break;
			default:
				LOG_ERROR("Unrecognised shader stage");
				return false;
			}
		}

		shader->data = new vulkan_shader_state();
		auto state = (vulkan_shader_state*)shader->data;
		state->renderpass = renderpass;
		state->configuration.max_descriptor_set_count = 1024;

		for (auto i{ 0U }; i < stages.size(); ++i)
		{
			auto& stage = stages[i];
			auto& name = stage_filenames[i];
			state->configuration.stages.emplace_back(stage, name);
		}

		state->configuration.pool_sizes.push_back(vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 1024));
		state->configuration.pool_sizes.push_back(vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 4096));

		vulkan_descriptor_set_configuration global_descriptor_set_configuration{};

		vk::DescriptorSetLayoutBinding ubo{};
		ubo
			.setBinding(BINDING_INDEX_UBO)
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);

		global_descriptor_set_configuration.bindings.push_back(ubo);

		state->configuration.descriptor_sets[DESCRIPTOR_SET_INDEX_GLOBAL] = global_descriptor_set_configuration;

		if (shader->has_instances())
		{
			vulkan_descriptor_set_configuration instance_descriptor_set_configuration{};
			vk::DescriptorSetLayoutBinding instance_ubo{};
			instance_ubo
				.setBinding(BINDING_INDEX_UBO)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);
		
			instance_descriptor_set_configuration.bindings.push_back(instance_ubo);
			state->configuration.descriptor_sets[DESCRIPTOR_SET_INDEX_INSTANCE] = instance_descriptor_set_configuration;
		}

		for (auto i{ 0U }; i < stages.size(); ++i)
		{
			state->stages.push_back(create_module(shader, state->configuration.stages[i]));
		}

		const auto& attributes = shader->get_attributes();

		uint32_t offset{ 0 };
		for (auto i{ 0U }; i < attributes.size(); ++i)
		{
			vk::VertexInputAttributeDescription attribute{};
			attribute
				.setLocation(i)
				.setBinding(0)
				.setOffset(offset)
				.setFormat(vulkan_attribute_types[attributes[i].type]);
			state->configuration.attributes.push_back(attribute);

			offset += attributes[i].size;
		}

		for (const auto& uniform : shader->get_uniforms())
		{
			if (uniform.type == shader_uniform_type::sampler)
			{
				const uint32_t set_index = uniform.scope == shader_scope::global ? DESCRIPTOR_SET_INDEX_GLOBAL : DESCRIPTOR_SET_INDEX_INSTANCE;
				auto& set_configuration = state->configuration.descriptor_sets[set_index];

				if (set_configuration.bindings.size() < 2)
				{
					vk::DescriptorSetLayoutBinding sampler{};
					sampler
						.setBinding(BINDING_INDEX_SAMPLER)
						.setDescriptorCount(1)
						.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
						.setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);
					set_configuration.bindings.push_back(sampler);
				}
				else
				{
					set_configuration.bindings[BINDING_INDEX_SAMPLER].descriptorCount++;
				}
			}
		}

		vk::DescriptorPoolCreateInfo pool_info{};
		pool_info
			.setPoolSizes(state->configuration.pool_sizes)
			.setMaxSets(state->configuration.max_descriptor_set_count)
			.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

		state->descriptor_pool = context_.device.logical_device.createDescriptorPool(pool_info, context_.allocator);

		for (auto i{ 0U }; i < state->configuration.descriptor_sets.size(); i++)
		{
			vk::DescriptorSetLayoutCreateInfo layout_info{};
			layout_info
				.setBindings(state->configuration.descriptor_sets[i].bindings);

			state->descriptor_set_layout[i] = context_.device.logical_device.createDescriptorSetLayout(layout_info, context_.allocator);
		}

		vk::Viewport viewport{};
		viewport
			.setX(0.F)
			.setY(context_.framebuffer_height)
			.setWidth(context_.framebuffer_width)
			.setHeight(-context_.framebuffer_height)
			.setMinDepth(0.F)
			.setMaxDepth(1.F);

		vk::Rect2D scissor{};
		scissor
			.setExtent({ context_.framebuffer_width, context_.framebuffer_height })
			.setOffset({ 0, 0 });

		egkr::vector<vk::PipelineShaderStageCreateInfo> stage_create_infos{};
		for (const auto& stage : state->stages)
		{
			stage_create_infos.push_back(stage.shader_stage_create_info);
		}

		vk::VertexInputBindingDescription binding_desc{};
		binding_desc
			.setBinding(0)
			.setStride(shader->get_attribute_stride())
			.setInputRate(vk::VertexInputRate::eVertex);

		pipeline_properties pipeline_properties{};
		pipeline_properties.renderpass = renderpass;
		pipeline_properties.descriptor_set_layout = state->descriptor_set_layout;
		pipeline_properties.shader_stage_info = stage_create_infos;
		pipeline_properties.is_wireframe = false;
		pipeline_properties.depth_test_enabled = true;
		pipeline_properties.scissor = scissor;
		pipeline_properties.viewport = viewport;
		pipeline_properties.push_constant_ranges = shader->get_push_constant_ranges();
		pipeline_properties.input_binding_description = binding_desc;
		pipeline_properties.input_attribute_description = state->configuration.attributes;

		state->pipeline = pipeline::create(&context_, pipeline_properties);

		shader->set_global_ubo_stride(get_aligned(shader->get_global_ubo_size(), 256));
		shader->set_ubo_stride(get_aligned(shader->get_ubo_size(), 256));

		state->uniform_buffer = buffer::create(&context_, shader->get_global_ubo_stride() + (shader->get_ubo_stride() * max_material_count), vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, true);
		state->mapped_uniform_buffer_memory = state->uniform_buffer->lock(0, VK_WHOLE_SIZE, 0);

		egkr::vector<vk::DescriptorSetLayout> global_layouts = { state->descriptor_set_layout[DESCRIPTOR_SET_INDEX_GLOBAL],state->descriptor_set_layout[DESCRIPTOR_SET_INDEX_GLOBAL],state->descriptor_set_layout[DESCRIPTOR_SET_INDEX_GLOBAL] };

		vk::DescriptorSetAllocateInfo alloc_info{};
		alloc_info
			.setDescriptorPool(state->descriptor_pool)
			.setSetLayouts(global_layouts);

		state->global_descriptor_sets = context_.device.logical_device.allocateDescriptorSets(alloc_info);
		return false;
	}

	void renderer_vulkan::free_shader(shader* /*shader*/)
	{

	}

	bool renderer_vulkan::use_shader(shader* shader)
	{
		auto state = (vulkan_shader_state*)shader->data;

		state->pipeline->bind(context_.graphics_command_buffers[context_.image_index], vk::PipelineBindPoint::eGraphics);
		return true;
	}

	bool renderer_vulkan::bind_shader_globals(shader* shader)
	{
		shader->set_bound_ubo_offset(shader->get_global_ubo_offset());
		return false;
	}

	bool renderer_vulkan::bind_shader_instances(shader* shader, uint32_t instance_id)
	{
		auto state = (vulkan_shader_state*)shader->data;
		shader->set_bound_instance_id(instance_id);

		shader->set_bound_ubo_offset(state->instance_states[instance_id].offset);
		return true;
	}

	bool renderer_vulkan::apply_shader_globals(shader* shader)
	{
		const auto image_index = context_.image_index;
		auto state = (vulkan_shader_state*)shader->data;
		auto& command_buffer = context_.graphics_command_buffers[image_index].get_handle();
		auto& global_descriptor = state->global_descriptor_sets[image_index];

		vk::DescriptorBufferInfo buffer_info{};
		buffer_info
			.setBuffer(state->uniform_buffer->get_handle())
			.setOffset(shader->get_global_ubo_offset())
			.setRange(shader->get_global_ubo_stride());

		vk::WriteDescriptorSet ubo_write{};
		ubo_write
			.setDstSet(global_descriptor)
			.setDstBinding(0)
			.setDstArrayElement(0)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1)
			.setBufferInfo(buffer_info);

		egkr::vector<vk::WriteDescriptorSet> writes{ ubo_write };

		context_.device.logical_device.updateDescriptorSets(writes, nullptr);
		command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, state->pipeline->get_layout(), 0, 1, &global_descriptor, 0, nullptr);

		return true;
	}

	bool renderer_vulkan::apply_shader_instances(shader* shader)
	{
		if (!shader->has_instances())
		{
			LOG_ERROR("This shader does not use instances");
			return false;
		}

		const auto image_index = context_.image_index;
		auto state = (vulkan_shader_state*)shader->data;
		auto& command_buffer = context_.graphics_command_buffers[image_index].get_handle();

		auto& object_state = state->instance_states[shader->get_bound_instance_id()];
		auto& object_descriptor_set = object_state.descriptor_set_state.descriptor_sets[image_index];

		egkr::vector<vk::WriteDescriptorSet> writes{};

		uint32_t descriptor_count{};
		uint32_t descriptor_index{};

		auto instance_ubo_generation = object_state.descriptor_set_state.descriptor_states[descriptor_index].generations[image_index];

		if (instance_ubo_generation == invalid_8_id)
		{
			vk::DescriptorBufferInfo buffer_info{};
			buffer_info
				.setBuffer(state->uniform_buffer->get_handle())
				.setOffset(object_state.offset)
				.setRange(shader->get_ubo_stride());

			vk::WriteDescriptorSet ubo{};
			ubo
				.setDstSet(object_descriptor_set)
				.setDstBinding(descriptor_index)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setDescriptorCount(1)
				.setBufferInfo(buffer_info);

			writes.push_back(ubo);
			descriptor_count++;

			instance_ubo_generation = 1;
		}
		++descriptor_index;

		if (state->configuration.descriptor_sets[DESCRIPTOR_SET_INDEX_INSTANCE].bindings.size() > 1)
		{
			auto total_sampler_count = state->configuration.descriptor_sets[DESCRIPTOR_SET_INDEX_INSTANCE].bindings[BINDING_INDEX_SAMPLER].descriptorCount;
			uint32_t update_sampler_count{};
			egkr::vector<vk::DescriptorImageInfo> image_infos{};

			for (auto i{ 0U }; i < total_sampler_count; ++i)
			{
				auto& texture = state->instance_states[shader->get_bound_instance_id()].instance_textures[i];
				auto texture_data = (vulkan_texture_state*)texture->data;

				vk::DescriptorImageInfo image_info{};
				image_info
					.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
					.setImageView(texture_data->image->get_view())
					.setSampler(texture_data->sampler);

				image_infos.push_back(image_info);

				++update_sampler_count;
			}

			vk::WriteDescriptorSet sampler{};
			sampler
				.setDstSet(object_descriptor_set)
				.setDstBinding(descriptor_index)
				.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
				.setDescriptorCount(update_sampler_count)
				.setImageInfo(image_infos);

			writes.push_back(sampler);
			++descriptor_count;

		}

		if (descriptor_count > 0)
		{
			context_.device.logical_device.updateDescriptorSets(writes, nullptr);
		}

		command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, state->pipeline->get_layout(), 1, object_descriptor_set, nullptr);
		return true;
	}

	uint32_t renderer_vulkan::acquire_shader_isntance_resources(shader* shader)
	{
		auto state = (vulkan_shader_state*)shader->data;

		auto instance_id = state->instance_states.size();

		vulkan_shader_instance_state instance_state;
		uint32_t instance_texture_count = state->configuration.descriptor_sets[DESCRIPTOR_SET_INDEX_INSTANCE].bindings[BINDING_INDEX_SAMPLER].descriptorCount;

		instance_state.instance_textures = egkr::vector<texture*>(instance_texture_count, texture_system::get_default_texture().get());

		vulkan_shader_descriptor_set_state set_state{};

		egkr::vector<vk::DescriptorSetLayout> layouts{ state->descriptor_set_layout[DESCRIPTOR_SET_INDEX_INSTANCE],state->descriptor_set_layout[DESCRIPTOR_SET_INDEX_INSTANCE],state->descriptor_set_layout[DESCRIPTOR_SET_INDEX_INSTANCE] };

		vk::DescriptorSetAllocateInfo alloc_info{};
		alloc_info
			.setDescriptorPool(state->descriptor_pool)
			.setSetLayouts(layouts);

		instance_state.descriptor_set_state.descriptor_sets = context_.device.logical_device.allocateDescriptorSets(alloc_info);

		instance_state.descriptor_set_state = set_state;
		state->instance_states.push_back(instance_state);
		return instance_id;
	}

	bool renderer_vulkan::set_uniform(shader* shader, const shader_uniform& uniform, const void* value)
	{
		auto internal = (vulkan_shader_state*)shader->data;
		if (uniform.type == shader_uniform_type::sampler)
		{
			if (uniform.scope == shader_scope::global)
			{
				shader->set_global_texture(uniform.location, (texture*)value);
			}
			else
			{
				internal->instance_states[shader->get_bound_instance_id()].instance_textures[uniform.location] = (texture*)value;
			}
		}
		else
		{
			if (uniform.scope == shader_scope::local)
			{
				// Is local, using push constants. Do this immediately.
				auto& command_buffer = context_.graphics_command_buffers[context_.image_index].get_handle();
				vkCmdPushConstants(command_buffer, internal->pipeline->get_layout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, uniform.offset, uniform.size, value);
			}
			else
			{
				// Map the appropriate memory location and copy the data over.
				auto addr = (uint64_t*)internal->mapped_uniform_buffer_memory;
				addr += shader->get_bound_ubo_offset() + uniform.offset;

				memcpy(addr, value, uniform.size);
				if (addr)
				{
				}
			}
		}
		return true;
	}

	vulkan_shader_stage renderer_vulkan::create_module(shader* /*shader*/, const vulkan_shader_stage_configuration& configuration)
	{
		auto resource = resource_system::load(configuration.filename, resource_type::binary);
		auto* code = (binary_resource_properties*)resource->data;

		vulkan_shader_stage shader_stage{};

		vk::ShaderModuleCreateInfo create_info{};
		create_info
			.setCodeSize(code->data.size())
			.setPCode((const uint32_t*)(code->data.data()));
		shader_stage.handle = context_.device.logical_device.createShaderModule(create_info, context_.allocator);
		shader_stage.create_info = create_info;

		resource_system::unload(resource);

		vk::PipelineShaderStageCreateInfo pipeline_create_info{};
		pipeline_create_info
			.setStage(configuration.stage)
			.setModule(shader_stage.handle)
			.setPName("main");

		shader_stage.shader_stage_create_info = pipeline_create_info;

		return shader_stage;
	}
}

