#include "bitmap_font_loader.h"

#include <format>
namespace egkr
{
	bitmap_font_loader::unique_ptr bitmap_font_loader::create(const loader_properties& properties)
	{
		return std::make_unique<bitmap_font_loader>(properties);
	}

	bitmap_font_loader::bitmap_font_loader(const loader_properties& properties)
		: resource_loader(resource::type::bitmap_font, properties)
	{}

	resource::shared_ptr bitmap_font_loader::load(std::string_view name, void* /*params*/)
	{
		auto base_path = get_base_path();

		std::string filename;

		std::array<supported_bitmap_font_file_type, 2> filetypes{ supported_bitmap_font_file_type{".ebf", bitmap_font_file_type::ebf, true},  supported_bitmap_font_file_type{".fnt", bitmap_font_file_type::fnt, false} };

		bool found{};
		supported_bitmap_font_file_type filetype{};
		for (const auto& extension : filetypes)
		{
			filename = std::format("{}/{}{}", base_path.data(), name.data(), extension.extension.data());

			if (filesystem::does_path_exist(filename))
			{
				filetype = extension;
				found = true;
				break;
			}

		}

		if (!found)
		{
			LOG_ERROR("File not found: {}", filename);
			return nullptr;
		}


		font::bitmap_font_resource_data resource_data{};
		auto file = filesystem::open(filename, file_mode::read, filetype.is_binary);

		switch (filetype.type)
		{
		case bitmap_font_file_type::fnt:
		{
			filename = std::format("{}/{}{}", base_path.data(), name.data(), ".ebf");

			std::string ebf_file_name = filename;
			resource_data = import_fnt_file(file, ebf_file_name);
			resource_data.font_data.font_type = font::type::bitmap;
		} break;
		case bitmap_font_file_type::ebf:
		{
			resource_data = read_ebf_file(file);
			resource_data.font_data.font_type = font::type::bitmap;
		} break;
		case bitmap_font_file_type::not_found:
		default:
		{
			LOG_WARN("Shouldn't have got here");
		}
		}

		resource::properties properties{ .resource_type = resource::type::bitmap_font, .name = name.data(), .full_path = filename,};
		properties.data = new font::bitmap_font_resource_data();
		*((font::bitmap_font_resource_data*)(properties.data)) = resource_data;
		return resource::create(properties);
	}

	bool bitmap_font_loader::unload(const resource::shared_ptr& resource)
	{
		if (resource->data)
		{
			free(resource->data);
			resource->data = nullptr;
		}
		return true;
	}
	
	font::bitmap_font_resource_data bitmap_font_loader::import_fnt_file(file_handle& handle, std::string_view ebf_filename)
	{
		font::bitmap_font_resource_data data{};

			uint32_t line_number{};
			egkr::vector<uint8_t> line{1};
		for (; !line.empty(); line = filesystem::read_line(handle, 511), ++line_number)
		{
			std::string line_string{ line.begin(), line.end() };
			trim(line_string);

			if (line[0] == '#' || line[0] == '\0')
			{
				continue;
			}

			auto first_char = line[0];
			switch (first_char)
			{
			case 'i' :
			{
				char buff[128]{};
				sscanf(line_string.data(), R"(info face="%[^"]" size=%u)", buff, &data.font_data.size);
				data.font_data.face = buff;
			} break;
			case 'c':
			{
				if (line[1] == 'o')
				{
					uint32_t num_pages{};
					sscanf(line_string.data(), "common lineHeight=%d base=%*ud scaleW=%ud scaleH=%ud pages=%u", &data.font_data.line_height, &data.font_data.atlas_size_x, &data.font_data.atlas_size_y, &num_pages);

				}
				else if (line[4] == 's')
				{
					uint32_t num_chars{};
					sscanf(line_string.data(), "chars count=%u", &num_chars);
				}
				else
				{
					font::glyph glyph{};
					sscanf(line_string.data(), "char id=%d x=%hu y=%hu width=%hu height=%hu xoffset=%hd yoffset=%hd xadvance=%hd page=%hhu", &glyph.codepoint, &glyph.x, &glyph.y, &glyph.width, &glyph.height, &glyph.x_offset, &glyph.y_offset, &glyph.x_advance, &glyph.page_id);
					data.font_data.glyphs.push_back(glyph);
				}
			} break;
			case 'p':
			{
				font::bitmap_font_page page{};
				page.file.resize(128);
				sscanf(line_string.data(), R"(page id=%hhi file="%[^"]")", &page.id, page.file.data());
				data.pages.push_back(page);
			} break;
			case 'k':
			{
				if (line[7] == 's')
				{
					uint32_t kerning_count{};
					sscanf(line_string.data(), "kernings count=%u", &kerning_count);
				}
				else
				{
					font::kerning kerning{};
					sscanf(line_string.data(), "kerning first=%d second=%d amount=%hd", &kerning.codepoint_0, &kerning.codepoint_1, &kerning.amount);
					data.font_data.kernings.push_back(kerning);
				}
			} break;
			default:
				LOG_ERROR("Shouldn't be here");
			}
		}
		return write_ebf_file(ebf_filename, data);
	}

	font::bitmap_font_resource_data bitmap_font_loader::read_ebf_file(file_handle& handle)
	{
		resource::header header{ };
		filesystem::read(handle, header);

		font::bitmap_font_resource_data data{};
		filesystem::read(handle, data.font_data.font_type);
		size_t face_length{};
		filesystem::read<size_t>(handle, face_length);

		char buff[128]{};
		filesystem::read(handle, buff, sizeof(const char), face_length);

		data.font_data.face = buff;
		filesystem::read(handle, data.font_data.size);
		filesystem::read(handle, data.font_data.line_height);
		filesystem::read(handle, data.font_data.baseline);
		filesystem::read(handle, data.font_data.atlas_size_x);
		filesystem::read(handle, data.font_data.atlas_size_y);

		size_t page_count{};
		filesystem::read<size_t>(handle, page_count);
		for (auto i{0U}; i < page_count; ++i)
		{
			font::bitmap_font_page page{};
			filesystem::read(handle, page.id);

			size_t file_length{};
			filesystem::read<size_t>(handle, file_length);
			page.file.resize(file_length);
			filesystem::read(handle, page.file.data(), sizeof(const char), page.file.size());

			data.pages.push_back(page);
		}

		size_t glyph_count{};
		filesystem::read<size_t>(handle, glyph_count);
		for (auto i{ 0U }; i < glyph_count; ++i)
		{
			font::glyph glyph{};
			filesystem::read(handle, glyph);
			data.font_data.glyphs.push_back(glyph);
		}

		size_t kerning_count{};
		filesystem::read<size_t>(handle, kerning_count);
		if (kerning_count > 0)
		{
			for (auto i{ 0U }; i < kerning_count; ++i)
			{
				font::kerning kerning{};
				filesystem::read(handle, kerning);
				data.font_data.kernings.push_back(kerning);
			}
		}
		filesystem::read(handle, data.font_data.tab_advance);

		return data;
	}

	font::bitmap_font_resource_data bitmap_font_loader::write_ebf_file(std::string_view path, const font::bitmap_font_resource_data& data)
	{
		file_handle handle = filesystem::open(path, file_mode::write, true);
		if (!handle.is_valid)
		{
			LOG_ERROR("Couldn't create ebf file");
			return {};
		}

		resource::header header{ .resource_type = resource::type::bitmap_font };
		filesystem::write(handle, header, 1);

		filesystem::write(handle, data.font_data.font_type);
		filesystem::write(handle, data.font_data.face.size());
		filesystem::write(handle, data.font_data.face.data(), sizeof(const char), data.font_data.face.size());
		filesystem::write(handle, data.font_data.size);
		filesystem::write(handle, data.font_data.line_height);
		filesystem::write(handle, data.font_data.baseline);
		filesystem::write(handle, data.font_data.atlas_size_x);
		filesystem::write(handle, data.font_data.atlas_size_y);
		
		filesystem::write(handle, data.pages.size());
		for (const auto& page : data.pages)
		{
			filesystem::write(handle, page.id);
			filesystem::write(handle, page.file.size());
			filesystem::write(handle, page.file.data(), sizeof(const char), page.file.size());
		}

		filesystem::write(handle, data.font_data.glyphs.size());
		for (const auto& glyph : data.font_data.glyphs)
		{
			filesystem::write(handle, &glyph, sizeof(font::glyph), 1);
		}

		filesystem::write(handle, data.font_data.kernings.size());
		if (data.font_data.kernings.size() > 0)
		{
			for (const auto& kerning : data.font_data.kernings)
			{
				filesystem::write(handle, &kerning, sizeof(font::kerning), 1);
			}
		}
		filesystem::write(handle, data.font_data.tab_advance);

		return data;
	}
}
