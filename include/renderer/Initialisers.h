#pragma once

#include "RendererCore.h"

namespace egkr::initialisers
{
	namespace pipeline
	{
		auto viewportCreate(float x, float y, vk::Extent2D swapchainExtent, float minDepth, float maxDepth, vk::Offset2D offset) -> vk::PipelineViewportStateCreateInfo;


		constexpr auto shaderCreate(vk::ShaderStageFlagBits shaderStage, std::string_view entry, vk::ShaderModule module) noexcept
		{
			vk::PipelineShaderStageCreateInfo shaderStageInfo{};
			return shaderStageInfo
				.setStage(shaderStage)
				.setPName(entry.data())
				.setModule(module);
		}

		auto vertexInputCreate() noexcept -> vk::PipelineVertexInputStateCreateInfo;


		consteval auto inputAssemblyCreate(vk::PrimitiveTopology topology, bool restartEnabled) noexcept
		{
			vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
			
			return inputAssembly
				.setTopology(topology)
				.setPrimitiveRestartEnable(restartEnabled);
		}

		consteval auto rasterizationCreate(bool depthClampEnabled, bool discardEnabled, vk::PolygonMode mode, float lineWidth, vk::CullModeFlagBits cullMode, vk::FrontFace frontFace, bool biasEnabled, float biadConstantFactor, float biasClamp, float biasSlopeFactor)
		{
			vk::PipelineRasterizationStateCreateInfo rasterizationCreate{};

			return rasterizationCreate
				.setDepthClampEnable(depthClampEnabled)
				.setRasterizerDiscardEnable(discardEnabled)
				.setPolygonMode(mode)
				.setLineWidth(lineWidth)
				.setCullMode(cullMode)
				.setFrontFace(frontFace)
				.setDepthBiasEnable(biasEnabled)
				.setDepthBiasConstantFactor(biadConstantFactor)
				.setDepthBiasClamp(biasClamp)
				.setDepthBiasSlopeFactor(biasSlopeFactor);
		}

		consteval auto multisampleCreate(bool sampleShadingEnabled, vk::SampleCountFlagBits samples, float minSampleShading, bool alphaCoverageEnabled, bool alphaToOneEnabled)
		{
			vk::PipelineMultisampleStateCreateInfo multisampleCreate{};

			return multisampleCreate
				.setSampleShadingEnable(sampleShadingEnabled)
				.setRasterizationSamples(samples)
				.setMinSampleShading(minSampleShading)
				.setPSampleMask(nullptr)
				.setAlphaToCoverageEnable(alphaCoverageEnabled)
				.setAlphaToOneEnable(alphaToOneEnabled);
		}

		auto colourBlendAttachementState(vk::ColorComponentFlags colourMask,
                  bool blendEnabled,
                  vk::BlendFactor srcColourBlend,
                  vk::BlendFactor dstColourBlend,
                  vk::BlendOp colourBlendOp,
                  vk::BlendFactor srcAlphaBlend,
                  vk::BlendFactor dstAlphaBlend,
                  vk::BlendOp alphaBlendOp) -> vk::PipelineColorBlendAttachmentState;

		consteval auto depthStencilCreate(bool enabled, bool writeEnabled, vk::CompareOp compareOp, bool boundsEnabled, bool stencilEnabled)
		{
			vk::PipelineDepthStencilStateCreateInfo depthStencilInfo{};

			return depthStencilInfo
				.setDepthTestEnable(enabled)
				.setDepthWriteEnable(writeEnabled)
				.setDepthCompareOp(compareOp)
				.setDepthBoundsTestEnable(boundsEnabled)
				.setStencilTestEnable(stencilEnabled);
		}

		auto colourBlendStateCreate(std::span<const vk::PipelineColorBlendAttachmentState> attachements,
                  bool logicOpEnabled,
                  vk::LogicOp logicOp,
                  const std::array<float, 4> &blendConstants) -> vk::PipelineColorBlendStateCreateInfo;

	}
}
