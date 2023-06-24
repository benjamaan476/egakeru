#include "renderer/SpriteRenderer.h"

#include "renderer/Renderer.h"
#include "renderer/RendererCore.h"
#include "renderer/RendererState.h"

#include "renderer/Initialisers.h"

namespace egkr
{
	void SpriteRenderer::create(vk::GraphicsPipelineCreateInfo basePipeline, vk::Extent2D swapchainExtent)
	{
		PROFILE_FUNCTION()

		vk::DescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding
			.setBinding(0)
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setStageFlags(vk::ShaderStageFlagBits::eFragment);

		auto bindings = { samplerLayoutBinding };
		vk::DescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.setBindings(bindings);

		_descriptorSetLayout = state.device.createDescriptorSetLayout(layoutInfo);
		ENGINE_ASSERT(_descriptorSetLayout != vk::DescriptorSetLayout{}, "Failed to create descriptor set");


		DESCRIPTOR_POOL(combinedSample, vk::DescriptorType::eCombinedImageSampler);

		auto descriptorPools = { combinedSample };

		vk::DescriptorPoolCreateInfo info{};
		info.setPoolSizes(descriptorPools);
		info.setMaxSets(MaxFramesInFlight * 2);
		descriptorPool = state.device.createDescriptorPool(info);
		ENGINE_ASSERT(descriptorPool != vk::DescriptorPool{}, "Failed to create desriptor pool");


		auto spriteVertShaderCode = readShader("spriteVert.spv");
		auto spriteFragShaderCode = readShader("spriteFrag.spv");

		vertShaderModule = egakeru::createShaderModule(spriteVertShaderCode);
		fragShaderModule = egakeru::createShaderModule(spriteFragShaderCode);


		auto bindingDescription = Vertex::getBindingDescription();
		auto attributeDescription = Vertex::getAttributeDescription();

		vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo
			.setVertexBindingDescriptions(bindingDescription)
			.setVertexAttributeDescriptions(attributeDescription);

		constexpr auto inputAssembly = initialisers::pipeline::inputAssemblyCreate(vk::PrimitiveTopology::eTriangleList, false);
		auto viewportState = initialisers::pipeline::viewportCreate(0.f, 0.f, swapchainExtent, 0.f, 1.f, { 0, 0 });

		constexpr auto rasterizer = initialisers::pipeline::rasterizationCreate(false, false, vk::PolygonMode::eFill, 1.f, vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise, false, 0.f, 0.f, 0.f);
		constexpr auto multisample = initialisers::pipeline::multisampleCreate(false, vk::SampleCountFlagBits::e1, 1.f, false, false);

		vk::PipelineShaderStageCreateInfo spriteVertShaderStageInfo{};
		spriteVertShaderStageInfo
			.setStage(vk::ShaderStageFlagBits::eVertex)
			.setModule(vertShaderModule)
			.setPName("main");

		vk::PipelineShaderStageCreateInfo spriteFragShaderStageInfo{};
		spriteFragShaderStageInfo
			.setStage(vk::ShaderStageFlagBits::eFragment)
			.setModule(fragShaderModule)
			.setPName("main");

		auto spriteShaderStages = { spriteVertShaderStageInfo, spriteFragShaderStageInfo };

		//vk::PipelineColorBlendAttachmentState colourBlenderAttachment2{};
		//colourBlenderAttachment2
		//	.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
		//	.setBlendEnable(true)
		//	.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
		//	.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
		//	.setColorBlendOp(vk::BlendOp::eAdd)
		//	.setSrcAlphaBlendFactor(vk::BlendFactor::eSrcAlpha)
		//	.setDstAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
		//	.setAlphaBlendOp(vk::BlendOp::eAdd);
		constexpr auto colourMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
		constexpr auto add = vk::BlendOp::eAdd;
		auto colourBlendAttachmentState = initialisers::pipeline::colourBlendAttachementState(colourMask, true, vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha, add, vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha, add);

		auto colourBlendAttachments = { colourBlendAttachmentState };
		auto colourBlendCreate = egkr::initialisers::pipeline::colourBlendStateCreate(colourBlendAttachments, false, vk::LogicOp::eCopy, { 1.f, 1.f, 1.f, 1.f });

		auto dynamicStates =
		{
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor
		};

		vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo{};

		dynamicStateCreateInfo.setDynamicStates(dynamicStates);

		constexpr auto depthStencilInfo = initialisers::pipeline::depthStencilCreate(true, true, vk::CompareOp::eLess, false, false);

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo
			.setPushConstantRanges(nullptr)
			.setSetLayouts(_descriptorSetLayout);

		pipelineLayout = state.device.createPipelineLayout(pipelineLayoutInfo);
		ENGINE_ASSERT(pipelineLayout != vk::PipelineLayout{}, "FAiled to create pipeline layout");

		vk::GraphicsPipelineCreateInfo spritePipelineInfo{};
		spritePipelineInfo
			.setStages(spriteShaderStages)
			.setPVertexInputState(&vertexInputInfo)
			.setPInputAssemblyState(&inputAssembly)
			.setPViewportState(&viewportState)
			.setPRasterizationState(&rasterizer)
			.setPMultisampleState(&multisample)
			.setPColorBlendState(&colourBlendCreate)
			.setPDynamicState(&dynamicStateCreateInfo)
			.setPDepthStencilState(&depthStencilInfo)
			.setLayout(pipelineLayout)
			.setRenderPass(basePipeline.renderPass)
			.setBasePipelineIndex(-1);

		auto createInfos = { spritePipelineInfo };

		auto pipelin = state.device.createGraphicsPipelines(VK_NULL_HANDLE, createInfos).value;
		spritePipeline = pipelin[0];

		vk::DeviceSize bufferSize = sizeof(egkr::Sprite::vertices[0]) * egkr::Sprite::vertices.size();

		BufferProperties properties =
		{
			.size = bufferSize,
			.usage = vk::BufferUsageFlagBits::eTransferSrc,
			.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostVisible
		};

		auto stagingBuffer = Buffer(properties);
		stagingBuffer.map(Sprite::vertices.data());

		BufferProperties vertexBufferProperties =
		{
			.size = bufferSize,
			.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
			.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostVisible
		};

		vertexBuffer = Buffer(vertexBufferProperties);

		vertexBuffer.copy(stagingBuffer);

		stagingBuffer.destroy();
	}

	std::shared_ptr<Sprite> SpriteRenderer::createSprite(glm::vec2 size, const Texture2D& texture)
	{
		PROFILE_FUNCTION()
		auto sprite = std::make_shared<egkr::Sprite>();
		sprite->size = size;
		sprite->texture = texture;


		//for (size_t i = 0; i < sprite.uboBuffer.size(); i++)
		{
			std::vector<vk::DescriptorSetLayout> layouts(MaxFramesInFlight, _descriptorSetLayout);

			vk::DescriptorSetAllocateInfo allocInfo{};
			allocInfo
				.setDescriptorPool(descriptorPool)
				.setDescriptorSetCount(MaxFramesInFlight)
				.setSetLayouts(layouts);

			sprite->descriptor = state.device.allocateDescriptorSets(allocInfo);
		}
		std::vector<vk::WriteDescriptorSet> writeDescriptorSets;

		writeDescriptorSets.resize(0);

		for (auto i = 0u; i < MaxFramesInFlight; i++)
		{

			vk::WriteDescriptorSet piecesDescriptorSet{};
			piecesDescriptorSet
				.setDstSet(sprite->descriptor[i])
				.setDstBinding(0)
				.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
				.setImageInfo(sprite->texture._imageInfo);

			writeDescriptorSets.push_back(piecesDescriptorSet);

		}

		state.device.updateDescriptorSets(writeDescriptorSets, nullptr);

		sprites.emplace_back(sprite);
		return sprite;
	}

	void SpriteRenderer::render(vk::CommandBuffer commandBuffer, uint32_t currentFrame)
	{
		PROFILE_FUNCTION()
		auto ortho = glm::ortho(0.f, 800.f, 600.f, 0.f, -1.f, 1.f);
		std::vector<Vertex> staging{};

		vk::DeviceSize bufferSize = sizeof(egkr::Sprite::vertices[0]) * egkr::Sprite::vertices.size() * sprites.size();

		BufferProperties properties =
		{
			.size = bufferSize,
			.usage = vk::BufferUsageFlagBits::eTransferSrc,
			.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostVisible
		};

		auto stagingBuffer = Buffer(properties);

		for (const auto& sprite : sprites)
		{
			auto model = glm::mat4(1.f);
			model = glm::translate(model, sprite->position);
			model = glm::translate(model, glm::vec3(0.5 * sprite->size.x, 0.5 * sprite->size.y, 0.f));
			model = glm::rotate(model, glm::radians(sprite->rotation), glm::vec3(0.f, 0.f, 1.f));
			model = glm::translate(model, glm::vec3(-0.5f * sprite->size.x, -0.5f * sprite->size.y, 0.f));
			model = glm::scale(model, glm::vec3(sprite->size, 1.f));
			//model = glm::translate(model, glm::vec3(-0.5f * sprite.size.x, -0.5f * sprite.size.y, 0.0f));
			{
				Vertex vert{};
				for (auto i = 0u; i < 4; i++)
				{
					vert = Sprite::vertices[i];
					vert.pos = ortho * model * Sprite::vertices[i].pos;
					vert.tex = Sprite::vertices[i].tex * glm::vec2(128.f / sprite->texture.getWidth(), 128.f / sprite->texture.getHeight());
					staging.emplace_back(vert);
				}
			}
		}

		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, spritePipeline);

		for (auto& sprite : sprites)
		{
			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, sprite->descriptor[currentFrame], nullptr);
		}

		stagingBuffer.map(staging.data());
		vertexBuffer.copy(stagingBuffer);

		vk::DeviceSize offsets = { 0 };
		commandBuffer.bindVertexBuffers(0, vertexBuffer.buffer, offsets);
		commandBuffer.drawIndexed(6, 1, 0, 0, 0);

		stagingBuffer.destroy();
	}

	void SpriteRenderer::destroy()
	{
		PROFILE_FUNCTION()

		for (auto& sprite : sprites)
		{
			sprite->destory();
		}
		state.device.destroyShaderModule(vertShaderModule);
		state.device.destroyShaderModule(fragShaderModule);
		state.device.destroyPipelineLayout(pipelineLayout);
		state.device.destroyPipeline(spritePipeline);
		state.device.destroyDescriptorSetLayout(_descriptorSetLayout);
		state.device.destroyDescriptorPool(descriptorPool);
		vertexBuffer.destroy();
	}
}