#include "renderer/Initialisers.h"

auto egkr::initialisers::pipeline::viewportCreate(float x, float y, vk::Extent2D swapchainExtent, float minDepth, float maxDepth, vk::Offset2D offset) -> vk::PipelineViewportStateCreateInfo
{
	vk::Viewport viewport{};
	viewport
		.setX(x)
		.setY(y)
		.setWidth((float)swapchainExtent.width)
		.setHeight((float)swapchainExtent.height)
		.setMinDepth(minDepth)
		.setMaxDepth(maxDepth);

	vk::Rect2D scissor{};
	scissor
		.setOffset(offset)
		.setExtent(swapchainExtent);

	vk::PipelineViewportStateCreateInfo viewportCreate{};
	return viewportCreate
		.setViewports(viewport)
		.setScissors(scissor);
}

auto egkr::initialisers::pipeline::vertexInputCreate() noexcept -> vk::PipelineVertexInputStateCreateInfo
{

	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescription = Vertex::getAttributeDescription();


	vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};

	return vertexInputInfo
		.setVertexBindingDescriptions(bindingDescription)
		//.setPVertexBindingDescriptions(&bindingDescription);
		.setVertexAttributeDescriptions(attributeDescription);
	//.setVertexAttributeDescriptionCount((uint32_t)attributeDescription.size())
	//.setPVertexAttributeDescriptions(attributeDescription.data());
}

auto egkr::initialisers::pipeline::colourBlendAttachementState(vk::ColorComponentFlags colourMask,
  bool blendEnabled,
  vk::BlendFactor srcColourBlend,
  vk::BlendFactor dstColourBlend,
  vk::BlendOp colourBlendOp,
  vk::BlendFactor srcAlphaBlend,
  vk::BlendFactor dstAlphaBlend,
  vk::BlendOp alphaBlendOp) -> vk::PipelineColorBlendAttachmentState
{
        vk::PipelineColorBlendAttachmentState colourBlendState{};

        return colourBlendState.setColorWriteMask(colourMask)
          .setBlendEnable(blendEnabled)
          .setSrcColorBlendFactor(srcColourBlend)
          .setDstColorBlendFactor(dstColourBlend)
          .setColorBlendOp(colourBlendOp)
          .setSrcAlphaBlendFactor(srcAlphaBlend)
          .setDstAlphaBlendFactor(dstAlphaBlend)
          .setAlphaBlendOp(alphaBlendOp);
}

auto egkr::initialisers::pipeline::colourBlendStateCreate(
  std::span<const vk::PipelineColorBlendAttachmentState> attachements,
  bool logicOpEnabled,
  vk::LogicOp logicOp,
  const std::array<float, 4> &blendConstants) -> vk::PipelineColorBlendStateCreateInfo
{
        vk::PipelineColorBlendStateCreateInfo colourBlendState{};

        return colourBlendState.setAttachmentCount((uint32_t)attachements.size())
          .setPAttachments(attachements.data())
          .setLogicOpEnable(logicOpEnabled)
          .setLogicOp(logicOp)
          .setBlendConstants(blendConstants);
}