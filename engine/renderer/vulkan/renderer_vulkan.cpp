#include "renderer_vulkan.h"
#include "vulkan_types.h"

#include "swapchain.h"

#include "vulkan_geometry.h"
#include "vulkan_shader.h"
#include "vulkan_render_target.h"
#include "resources/shader.h"

namespace egkr
{
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
		const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
		void* /*userData*/)
	{
		ZoneScoped;

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
			break;
		}

		return (VkBool32)true;

	}

	uint32_t vulkan_device::find_memory_index(uint32_t type_filter, vk::MemoryPropertyFlags property_flags) const
	{
		ZoneScoped;

		const auto device_properties = physical_device.getMemoryProperties();

		for (auto i{ 0U }; i < device_properties.memoryTypeCount; ++i)
		{
			if (type_filter & (1 << i) && (device_properties.memoryTypes[i].propertyFlags & property_flags) == property_flags)
			{
				return i;
			}
		}

		LOG_WARN("Failed to find suitable memory type");
		return invalid_32_id;
	}

	bool vulkan_device::physical_device_meets_requirements(
		vk::PhysicalDevice device,
		vk::SurfaceKHR surface,
		const vk::PhysicalDeviceProperties& device_properties,
		const vk::PhysicalDeviceFeatures device_features,
		const physical_device_requirements& requirements,
		physical_device_queue_family_info& family_info,
		swapchain_support_details& swapchain_support)
	{
		ZoneScoped;

		if (requirements.discrete_gpu)
		{
			if (device_properties.deviceType != vk::PhysicalDeviceType::eDiscreteGpu)
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
				 family_info.graphics_index != invalid_32_id,
				 family_info.present_index != invalid_32_id,
				 family_info.compute_index != invalid_32_id,
				 family_info.transfer_index != invalid_32_id,
				 device_properties.deviceName.data());

		if (
			(!requirements.graphics || (requirements.graphics && family_info.graphics_index != invalid_32_id)) &&
			(!requirements.present || (requirements.present && family_info.present_index != invalid_32_id)) &&
			(!requirements.compute || (requirements.compute && family_info.compute_index != invalid_32_id)) &&
			(!requirements.transfer || (requirements.transfer && family_info.transfer_index != invalid_32_id)))
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

		if (requirements.sampler_anisotropy && !device_features.samplerAnisotropy)
		{
			LOG_INFO("Device does not support samplerAnisotropy, skipping");
			return false;
		}

		return true;
	}

	bool vulkan_device::select_physical_device(vulkan_context* context)
	{
		ZoneScoped;

		auto physical_devices = context->instance.enumeratePhysicalDevices();

		for (auto& device : physical_devices)
		{
			const auto device_properties = device.getProperties();
			const auto features = device.getFeatures();
			const auto memory = device.getMemoryProperties();

			physical_device_requirements requirements{};
			requirements.graphics = true;
			requirements.present = true;
			requirements.transfer = true;
			requirements.sampler_anisotropy = true;

			requirements.discrete_gpu = false;

			physical_device_queue_family_info queue_info{};

			auto result = physical_device_meets_requirements(device, context->surface, device_properties, features, requirements, queue_info, context->device.swapchain_supprt);

			if (result)
			{
				LOG_INFO("Selected device: {}", device_properties.deviceName.data());
				switch (device_properties.deviceType)
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

				LOG_INFO("GPU Driver version: {}.{}.{}", VK_VERSION_MAJOR(device_properties.driverVersion), VK_VERSION_MINOR(device_properties.driverVersion), VK_VERSION_PATCH(device_properties.driverVersion));
				LOG_INFO("Vulkan API version: {}.{}.{}", VK_VERSION_MAJOR(device_properties.apiVersion), VK_VERSION_MINOR(device_properties.apiVersion), VK_VERSION_PATCH(device_properties.apiVersion));

				context->device.physical_device = device;
				context->device.graphics_queue_index = queue_info.graphics_index;
				context->device.present_queue_index = queue_info.present_index;
				context->device.transfer_queue_index = queue_info.transfer_index;

				context->device.properties = device_properties;
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
		ZoneScoped;

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
		ZoneScoped;

		if (context_.graphics_command_buffers.empty())
		{
			context_.graphics_command_buffers.resize(context_.swpchain->get_image_count());
		}

		for (auto& command_buffer : context_.graphics_command_buffers)
		{
			if (command_buffer.get_handle())
			{
				command_buffer.free(&context_, context_.device.graphics_command_pool);
			}

			command_buffer.allocate(&context_, context_.device.graphics_command_pool, true);
		}
	}

	renderer_backend::unique_ptr renderer_vulkan::create(const platform::shared_ptr& platform)
	{
		ZoneScoped;

		return std::make_unique<renderer_vulkan>(platform);
	}

	renderer_vulkan::renderer_vulkan(platform::shared_ptr platform)
		: platform_{ std::move(platform) }
	{
		ZoneScoped;

		const auto size = platform_->get_framebuffer_size();
		context_.framebuffer_width = size.x;
		context_.framebuffer_height = size.y;
	}

	renderer_vulkan::~renderer_vulkan()
	{
		shutdown();
	}

	bool renderer_vulkan::init(const configuration& renderer_configurationn, uint8_t& out_window_attachment_count)
	{
		application_name_ = renderer_configurationn.application_name;
		ZoneScoped;

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
		context_.swpchain = swapchain::create(&context_, {renderer_configurationn.backend_flags});
		out_window_attachment_count = context_.swpchain->get_image_count();


		create_command_buffers();

		context_.image_available_semaphore.resize(context_.swpchain->get_max_frames_in_flight());
		context_.queue_complete_semaphore.resize(context_.swpchain->get_max_frames_in_flight());

		context_.in_flight_fences.resize(context_.swpchain->get_image_count());
		context_.images_in_flight.resize(context_.swpchain->get_image_count());

		for (auto i{ 0U }; i < context_.swpchain->get_max_frames_in_flight(); ++i)
		{
			context_.image_available_semaphore[i] = context_.device.logical_device.createSemaphore({}, context_.allocator);
			context_.queue_complete_semaphore[i] = context_.device.logical_device.createSemaphore({}, context_.allocator);

			context_.in_flight_fences[i] = fence::create(&context_, true);
		}

		return true;
	}

	void renderer_vulkan::shutdown()
	{
		ZoneScoped;

		if (context_.instance)
		{
			context_.device.logical_device.waitIdle();

			for (auto i{ 0U }; i < context_.swpchain->get_max_frames_in_flight(); ++i)
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

			context_.swpchain->destroy();
			context_.swpchain.reset();

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

	void renderer_vulkan::tidy_up()
	{
		if (context_.instance)
		{
			context_.device.logical_device.waitIdle();
		}
	}

	void renderer_vulkan::resize(uint32_t width, uint32_t height)
	{
		ZoneScoped;

		context_.framebuffer_width = width;
		context_.framebuffer_height = height;

		++context_.framebuffer_size_generation;

		if (context_.framebuffer_size_generation != context_.framebuffer_last_size_generation)
		{
			context_.device.logical_device.waitIdle();
			recreate_swapchain();
		}
	}

	bool renderer_vulkan::prepare_frame(frame_data& frame_data)
	{
		frame_number_++;
		draw_index_ = 0;
		if (context_.recreating_swapchain)
		{
			context_.device.logical_device.waitIdle();
			return false;
		}
		context_.in_flight_fences[context_.current_frame]->wait(std::numeric_limits<uint64_t>::max());
		context_.image_index = context_.swpchain->acquire_next_image_index(context_.image_available_semaphore[context_.current_frame], VK_NULL_HANDLE);

		frame_data.frame_number = frame_number_;
		frame_data.draw_index = draw_index_;
		frame_data.render_target_index = get_window_index();

		return true;
	}

	bool renderer_vulkan::begin(const frame_data& /*frame_data*/)
	{
		ZoneScoped;

		context_.in_flight_fences[context_.current_frame]->wait(std::numeric_limits<uint64_t>::max());
		auto& command_buffer = context_.graphics_command_buffers[context_.image_index];
		command_buffer.reset();
		command_buffer.begin(false, false, false);

		return true;
	}

	void renderer_vulkan::end(frame_data& frame_data)
	{
		ZoneScoped;

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
			.setWaitDstStageMask(stage_mask);
		if (draw_index_ == 0)
		{
			submit_info
				.setSignalSemaphores(context_.queue_complete_semaphore[context_.current_frame])
				.setWaitSemaphores(context_.image_available_semaphore[context_.current_frame]);
		}
		else
		{
			submit_info
				.setSignalSemaphoreCount(0)
				.setWaitSemaphoreCount(0);
		}

		context_.device.graphics_queue.submit(submit_info, context_.in_flight_fences[context_.current_frame]->get_handle());
		command_buffer.update_submitted();
		draw_index_++;
		frame_data.draw_index++;

		FrameMark;
	}

	void renderer_vulkan::present(const frame_data& /*frame_data*/)
	{
		context_.swpchain->present(context_.device.graphics_queue, context_.device.present_queue, context_.queue_complete_semaphore[context_.current_frame], context_.image_index);
	}

	bool renderer_vulkan::init_instance()
	{
		ZoneScoped;

		vk::ApplicationInfo application_info{};
		application_info
			.setPApplicationName("engine")
			.setApplicationVersion(VK_MAKE_API_VERSION(0, 1, 3, 0))
			.setPEngineName("egakeru")
			.setEngineVersion(VK_MAKE_API_VERSION(0, 1, 3, 0))
			.setApiVersion(VK_API_VERSION_1_3);

		vk::InstanceCreateInfo instance_info{};
		instance_info.setPApplicationInfo(&application_info);

		auto extensions = platform_->get_required_extensions();

		if (enable_validation_layers_)
		{
			LOG_INFO("Adding debug validation");
 			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		instance_info.setEnabledExtensionCount((uint32_t)extensions.size());
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

#ifdef ENABLE_DEBUG_MACRO
		context_.pfn_set_debug_name = (PFN_vkSetDebugUtilsObjectNameEXT)context_.instance.getProcAddr("vkSetDebugUtilsObjectNameEXT");
#endif // ENABLE_DEBUG_MACRO


		LOG_INFO("Vulkan instance initialised");
		return true;
	}

	bool renderer_vulkan::create_debug_messenger()
	{
		ZoneScoped;

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
		ZoneScoped;

		return platform_->create_surface(context_.instance);
	}

	bool renderer_vulkan::pick_physical_device()
	{
		ZoneScoped;

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
		ZoneScoped;

		auto queueIndices = find_queue_families(context_, device);

		auto properties = device.getProperties();
		auto features = device.getFeatures();

		auto swapChainSupport = query_swapchain_support(context_.surface, device);
		auto swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.present_modes.empty();

		return (properties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu || properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) && (bool)features.geometryShader && queueIndices.is_complete() && swapChainAdequate;
	}

	bool renderer_vulkan::check_device_extension_support(const vk::PhysicalDevice& physical_device)
	{
		ZoneScoped;

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
		ZoneScoped;

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
		ZoneScoped;

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
		for (auto i{ 0U }; i < context_.swpchain->get_image_count(); ++i)
		{
			context_.graphics_command_buffers[i].free(&context_, context_.device.graphics_command_pool);
		}

		context_.swpchain->recreate();

		create_command_buffers();
		context_.recreating_swapchain = false;
		return true;
	}

	bool renderer_vulkan::is_multithreaded() const
	{
		return context_.multithreading_enabled;
	}

	void renderer_vulkan::free_material(material* material) const
	{
		ZoneScoped;

		material->get_diffuse_map()->release();
		material->get_specular_map()->release();
		material->get_normal_map()->release();
	}

	texture* renderer_vulkan::create_texture() const
	{
		return new vulkan_texture();
	}

	texture* renderer_vulkan::create_texture(const texture::properties& properties, const uint8_t* data) const
	{
		ZoneScoped;

		auto tex = vulkan_texture::create(&context_, properties.width, properties.height, properties, true);
		tex->populate(properties, data);

		SET_DEBUG_NAME(&context_, VkObjectType::VK_OBJECT_TYPE_IMAGE, (uint64_t)(VkImage)tex->get_image(), properties.name)
		SET_DEBUG_NAME(&context_, VkObjectType::VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)(VkImageView)tex->get_view(), properties.name + "_view")
		return tex;
	}

	void renderer_vulkan::create_texture(const texture::properties& properties, const uint8_t* data, texture* out_texture) const
	{
		auto tex = vulkan_texture::create(&context_, properties.width, properties.height, properties, true);
		tex->populate(properties, data);

		SET_DEBUG_NAME(&context_, VkObjectType::VK_OBJECT_TYPE_IMAGE, (uint64_t)(VkImage)tex->get_image(), properties.name)
		SET_DEBUG_NAME(&context_, VkObjectType::VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)(VkImageView)tex->get_view(), properties.name + "_view")
		*(vulkan_texture*)out_texture = *tex;
	}

	shader::shared_ptr renderer_vulkan::create_shader(const shader::properties& properties) const
	{
		ZoneScoped;

		return vulkan_shader::create(&context_, properties);
	}

	geometry::shared_ptr renderer_vulkan::create_geometry(const geometry::properties& properties) const
	{
		ZoneScoped;

		return vulkan_geometry::create(&context_, properties);
	}

	render_target::render_target::shared_ptr renderer_vulkan::create_render_target(const egkr::vector<render_target::attachment>& attachments, renderpass::renderpass* pass, uint32_t width, uint32_t height) const
	{
		return render_target::vulkan_render_target::create(&context_, attachments, pass, width, height);
	}

	render_target::render_target::shared_ptr renderer_vulkan::create_render_target(const egkr::vector<render_target::attachment_configuration>& attachments) const
	{
		return render_target::vulkan_render_target::create(&context_, attachments);
	}
	
	renderpass::renderpass::shared_ptr renderer_vulkan::create_renderpass(const renderpass::configuration& pass_configuration) const
	{
		return renderpass::vulkan_renderpass::create(&context_, pass_configuration);
	}

	texture_map::shared_ptr renderer_vulkan::create_texture_map(const texture_map::properties& properties) const
	{
		return vulkan_texture_map::create(&context_, properties);
	}

	renderbuffer::renderbuffer::shared_ptr renderer_vulkan::create_renderbuffer(renderbuffer::type buffer_type, uint64_t size) const
	{
		return vulkan_buffer::create(&context_, buffer_type, size);
	}

	void renderer_vulkan::set_viewport(const float4& rect) const
	{
		vk::Viewport viewport{};
		viewport
			.setX(rect.x)
			.setY(rect.y)
			.setWidth(rect.z)
			.setHeight(rect.w)
			.setMinDepth(0.F)
			.setMaxDepth(1.F);

		auto& command_buffer = context_.graphics_command_buffers[context_.image_index];
		command_buffer.get_handle().setViewport(0, viewport);
	}

	void renderer_vulkan::set_scissor(const float4& rect) const
	{
		vk::Rect2D scissor{};
		scissor
			.setOffset({ (int32_t)rect.x, (int32_t)rect.y })
			.setExtent({ (uint32_t)rect.z, (uint32_t)rect.w });

		auto& command_buffer = context_.graphics_command_buffers[context_.image_index];
		command_buffer.get_handle().setScissor(0, scissor);
	}

	void renderer_vulkan::reset_scissor() const
	{
		set_scissor(context_.scissor_rect);
	}

	void renderer_vulkan::set_winding(winding winding) const
	{
		auto& command_buffer = context_.graphics_command_buffers[context_.image_index];
		command_buffer.get_handle().setFrontFace(winding == winding::counter_clockwise ? vk::FrontFace::eCounterClockwise : vk::FrontFace::eClockwise);
	}

	texture* renderer_vulkan::get_window_attachment(uint8_t index) const
	{
		if (index >= context_.swpchain->get_image_count())
		{
			LOG_ERROR("Invalid index");
			return nullptr;
		}
		return context_.swpchain->get_render_texture(index);
	}

	texture* renderer_vulkan::get_depth_attachment(uint8_t index) const
	{
		return context_.swpchain->get_depth_attachment(index);
	}

	uint8_t renderer_vulkan::get_window_index() const
	{
		return (uint8_t)context_.image_index;
	}

#ifdef ENABLE_DEBUG_MACRO
	bool renderer_vulkan::set_debug_obj_name(const vulkan_context* context, VkObjectType type, uint64_t handle, const std::string& name)
	{
		if (handle)
		{
			VkDebugUtilsObjectNameInfoEXT info{ .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT, .objectType = type, .objectHandle = handle, .pObjectName = name.data() };
			context->pfn_set_debug_name((VkDevice)context->device.logical_device, &info);
			return true;
		}
		return false;
	}
#endif
}
