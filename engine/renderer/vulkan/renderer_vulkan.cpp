#include "renderer_vulkan.h"
#include "vulkan_types.h"

#include "platform/windows/platform_windows.h"
#include "swapchain.h"

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
			return false;
		}

		if (!create_debug_messenger())
		{
			return false;
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

		return true;
	}

	void renderer_vulkan::shutdown()
	{
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

	}
	void renderer_vulkan::resize(uint32_t /*width*/, uint32_t /*height*/)
	{
	}
	void renderer_vulkan::begin_frame(std::chrono::milliseconds /*delta_time*/)
	{
		++frame_number_;
	}
	void renderer_vulkan::end_frame()
	{
	}
	bool renderer_vulkan::init_instance()
	{
		vk::ApplicationInfo application_info{};
		application_info
			.setPApplicationName("engine")
			.setApplicationVersion(VK_MAKE_API_VERSION(0, 1, 0, 0))
			.setPEngineName("egakeru")
			.setEngineVersion(VK_MAKE_API_VERSION(0, 1, 0, 0))
			.setApiVersion(VK_API_VERSION_1_0);

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

		context_.device.logical_device = context_.device.physical_device.createDevice(device_create_info, context_.allocator);
		if (context_.device.logical_device == vk::Device{})
		{
			LOG_ERROR("Could not create logical device");
			return false;
		}

		return true;
	}


}