#pragma once

#include <resources/font.h>
#include <platform/filesystem.h>
#include <loaders/resource_loader.h>

namespace egkr
{
	enum class system_font_file_type
	{
		not_found,
		esf,
		font_config
	};

	struct supported_system_font_file_type
	{
		std::string extension{};
		system_font_file_type type{};
		bool is_binary{};
	};

	class system_font_loader : public resource_loader
	{
	public:
		using unique_ptr = std::unique_ptr<system_font_loader>;
		static unique_ptr create(const loader_properties& properties);

		explicit system_font_loader(const loader_properties& properties);

		resource::shared_ptr load(std::string_view name, void* params) override;
		bool unload(const resource::shared_ptr& resource) override;

	private:
		font::system_font_resource_data import_fontcfg_file(file_handle& handle, std::string_view esf_filename);
		static font::system_font_resource_data read_esf_file(file_handle& handle);
		static font::system_font_resource_data write_esf_file(std::string_view path, const font::system_font_resource_data& data);
	};
}
