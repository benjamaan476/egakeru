#pragma once

#include "resource_loader.h"
#include "resources/shader.h"

namespace egkr
{
	class shader_loader : public resource_loader
	{
	public:
		using unique_ptr = std::unique_ptr<shader_loader>;
		static unique_ptr create(const loader_properties& properties);

		explicit shader_loader(const loader_properties& properties);
		~shader_loader() override = default;

		resource::shared_ptr load(std::string_view name) override;
		bool unload(const resource::shared_ptr& resource) override;

	private:
		shader_properties load_configuration_file(std::string_view path);
	};
}
