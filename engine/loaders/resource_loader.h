#pragma once

#include "resources/resource.h"

namespace egkr
{
	struct loader_properties
	{
		std::string path;
		std::optional<std::string> custom_type;
	};

	class resource_loader
	{
	public:
		using unique_ptr = std::unique_ptr<resource_loader>;

		resource_loader(resource::type type, const loader_properties& properties);
		virtual ~resource_loader() = default;

		// Params is used to pass loader specific settings to the loader
		virtual resource::shared_ptr load(const std::string& name, void* params) = 0;
		virtual bool unload(const resource::shared_ptr& resource) = 0;

		[[nodiscard]] const auto& get_loader_type() const { return loader_type_; }

	protected:
		[[nodiscard]] std::string_view get_base_path() const { return type_path_; }
	private:
		resource::type loader_type_{};

		std::string custom_type_name_;
		std::string type_path_;
	};
}
