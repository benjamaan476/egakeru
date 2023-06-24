#pragma once

#include "RendererState.h"

#include "Buffer.h"
#include "CommandBuffer.h"
#include "RendererState.h"
#include "Texture.h"
#include "Vertex.h"

#include "../ApplicationCore.h"

#include <fstream>
namespace egkr
{
	const static constexpr uint32_t MaxFramesInFlight = 2u;
	constexpr uint32_t DescriptorCount = 1000;

#define DESCRIPTOR_POOL(name, type)			\
	vk::DescriptorPoolSize name{};			\
	name									\
		.setType(type) 						\
		.setDescriptorCount(DescriptorCount)

	std::vector<char> readShader(const std::filesystem::path& filePath);
}