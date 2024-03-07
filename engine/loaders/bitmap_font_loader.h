#pragma once

#include <resources/font.h>
#include <platform/filesystem.h>
#include <loaders/resource_loader.h>

namespace egkr
{
	enum class bitmap_font_file_type
	{
		not_found,
		ebf,
		fnt
	};

	struct supported_bitmap_font_file_type
	{
		std::string extension{};
		bitmap_font_file_type type{};
		bool is_binary{};
	};

	class bitmap_font_loader : public resource_loader
	{
	public:
		using unique_ptr = std::unique_ptr<bitmap_font_loader>;
		static unique_ptr create(const loader_properties& properties);

		explicit bitmap_font_loader(const loader_properties& properties);

		resource::shared_ptr load(std::string_view name, void* params) override;
		bool unload(const resource::shared_ptr& resource) override;

	private:
		static font::bitmap_font_resource_data import_fnt_file(file_handle& handle, std::string_view ebf_filename);
		static font::bitmap_font_resource_data read_ebf_file(file_handle& handle);
		static font::bitmap_font_resource_data write_ebf_file(std::string_view path, const font::bitmap_font_resource_data& data);
	};
}
