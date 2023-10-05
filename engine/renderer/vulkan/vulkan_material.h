#pragma once
#include "pch.h"

#include "resources/material.h"

namespace egkr
{
	struct vulkan_context;
	class vulkan_material : public material
	{
	public:
		using shared_ptr = std::shared_ptr <vulkan_material>;
		static shared_ptr create(const vulkan_context* context);

		explicit vulkan_material(const vulkan_context* context);
		~vulkan_material();

		void destroy();


	private:
		const vulkan_context* context_{};

	};
}
