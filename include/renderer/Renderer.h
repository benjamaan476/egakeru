#pragma once

#include "RendererCore.h"
#include "Sprite.h"
#include "SpriteRenderer.h"

#include "../BoardProperties.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace egkr
{
	class egakeru
	{
	public:
		static void create();
		static void cleanup();
		static void drawFrame(const BoardProperties& boardProperties);

		static egakeru& get() { return *_instance; }
		static vk::Format getFormat() { return _swapchainFormat; }
		static const std::vector<Texture2D>& getSwapchainImages() { return _swapchainImages; }
		static const inline vk::Sampler& getTextureSampler() { return textureSampler; }
		static const inline vk::Sampler getSampler() { return textureSampler; }
		static const inline vk::PipelineLayout getPipelineLayout() { return pipelineLayout; }

		static std::shared_ptr<egkr::Sprite> createSprite(const Texture2D& texture);

		//static void createImage(std::string_view imagePath);
		static vk::ShaderModule createShaderModule(const std::vector<char>& code);
	private:

		static inline egakeru* _instance = nullptr;
		static inline Gui::SharedPtr _gui;

		egakeru() = default;

		struct QueueFamilyIndices
		{
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;

			bool isComplete() const
			{
				return graphicsFamily.has_value() && presentFamily.has_value();
			}
		};

		struct SwapChainSupportDetails
		{
			vk::SurfaceCapabilitiesKHR capabilities;
			std::vector<vk::SurfaceFormatKHR> formats;
			std::vector<vk::PresentModeKHR> presentModes;
		};

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
			void* userData);

		static bool isDeviceSuitable(const vk::PhysicalDevice& device);
		static bool checkDeviceExtensionSupport(const vk::PhysicalDevice& physicalDevice);
		static SwapChainSupportDetails querySwapChainSupport(const vk::PhysicalDevice& physicsalDevice);
		static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
		static vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
		static vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
		static void setupDebugMessenger();
		static bool checkValidationLayerSupport();
		static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* createInfo, const VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT* debugMessenger);
		static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* allocator);

		static QueueFamilyIndices findQueueFamiles(const vk::PhysicalDevice& device);
		static std::vector<Texture2D> getSwapchainImages(vk::SwapchainKHR swapchain);

		static bool hasStencilComponent(vk::Format format);

		static void createInstance();
		static void createSurface();
		static void pickPhysicalDevice();
		static void createLogicalDevice();
		static void createSwapchain();
		static void createRenderPass();
		static void createDescriptorSetLayout();
		static void createGraphicsPipeline();
		static void createFramebuffers();
		static void createCommandPool();
		static void createDepthResources();
		static void createTextureSampler();
		static void createVertexBuffer();
		static void createIndexBuffer();
		static void createUniformBuffers();
		static void createDescriptorPool();
		static void createDescriptorSets();
		static void createCommandBuffers();
		static void createSyncObjects();
		static void updateUniformBuffer(const BoardProperties& boardProperties, uint32_t currentImage);

		static void recreateSwapChain();
		static void cleanupSwapchain();

		static void initVulkan();

	private:

		static inline VkDebugUtilsMessengerEXT _debugMessenger;
		static inline vk::SurfaceKHR surface;
		static inline vk::Format _swapchainFormat;
		static inline vk::Extent2D swapchainExtent;
		static inline vk::SwapchainKHR _swapchain;
		static inline std::vector<Texture2D> _swapchainImages;
		static inline vk::ShaderModule vertShaderModule;
		static inline vk::ShaderModule fragShaderModule;
		static inline vk::RenderPass renderPass;
		static inline vk::DescriptorSetLayout descriptorSetLayout;
		static inline vk::DescriptorPool descriptorPool;
		static inline vk::PipelineLayout pipelineLayout;
		static inline std::vector<vk::Framebuffer> swapChainFramebuffers;
		static inline Buffer vertexBuffer;
		static inline Buffer indexBuffer;
		static inline std::vector<Buffer> boardPropertiesBuffer;
		static inline vk::Sampler textureSampler;
		static inline DepthImage depthImage;
		static inline CommandBuffer _commandBuffers;
		static inline std::vector<vk::Semaphore> imageAvailableSemaphores;
		static inline std::vector<vk::Semaphore> renderFinishedSemaphores;
		static inline std::vector<vk::Fence> inFlightFences;

		static inline struct
		{
			vk::Pipeline board;
			vk::Pipeline pieces;
		} pipelines;

		static inline struct
		{
			std::vector<vk::DescriptorSet> board;
		} descriptorSets;

		static const inline std::vector<Vertex> vertices
		{
			{{-0.5f, -0.5f, 0.5f, 1.f}, {1.f, 0.f, 0.f}, {1.f, 0.f} },
			{ {0.5f, -0.5f, 0.5f, 1.f}, { 0.f, 1.f, 0.f }, {0.f, 0.f}},
			{{0.5f, 0.5f, 0.5f, 1.f}, {0.f, 0.f, 1.f}, {0.f, 1.f}},
			{{-0.5f, 0.5f, 0.5f, 1.f}, {1.f, 1.f, 1.f}, {1.f, 1.f}},
		};

		static const inline std::vector<uint32_t> indices =
		{
			0, 1, 2, 2, 3, 0
		};

		static const inline std::vector<const char*> validationLayers
		{
			"VK_LAYER_KHRONOS_validation",
		};

		static const inline std::vector<const char*> deviceExtensions
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		static inline float4 clearColor{ 0.f, 0.f, 0.f, 1.f };

#ifdef NDEBUG
		static const inline bool enableValidationLayers = false;
#else
		static const inline bool enableValidationLayers = true;
#endif

		static inline uint32_t currentFrame;
	};
}