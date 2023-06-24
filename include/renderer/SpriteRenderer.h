#pragma once

#include "../EngineCore.h"

#include "Sprite.h"

namespace egkr
{
	class SpriteRenderer
	{
		static inline vk::DescriptorPool descriptorPool{};
		static inline vk::DescriptorSetLayout _descriptorSetLayout{};
		static inline vk::Pipeline spritePipeline{};
		static inline Buffer vertexBuffer{};
		static inline vk::PipelineLayout pipelineLayout{};

		static inline vk::ShaderModule fragShaderModule{};
		static inline vk::ShaderModule vertShaderModule{};
		static inline std::vector<std::shared_ptr<Sprite>> sprites{};

	public:
		SpriteRenderer() noexcept {}
		static void create(vk::GraphicsPipelineCreateInfo basePipeline, vk::Extent2D swapchainExtent);

		static std::shared_ptr<Sprite> createSprite(glm::vec2 size, const Texture2D& texture);

		static void render(vk::CommandBuffer commandBuffer, uint32_t currentFrame);


		static void destroy();

	};

}