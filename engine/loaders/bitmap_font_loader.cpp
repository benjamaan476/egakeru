#include "bitmap_font_loader.h"

namespace egkr
{
	bitmap_font_loader::unique_ptr create(const loader_properties& properties)
	{
		return std::make_unique<bitmap_font_loader>(properties);
	}

	bitmap_font_loader::bitmap_font_loader(const loader_properties& properties)
		: resource_loader(resource_type::bitmap_font, properties)
	{}

	resource::shared_ptr bitmap_font_loader::load(std::string_view name, void* /*params*/)
	{
		auto base_path = get_base_path();
		constexpr std::string_view format_string{ "%s/%s/%s%s" };

		char buff[128];

		std::array<supported_bitmap_font_file_type, 2> filetypes{ supported_bitmap_font_file_type{".ebf", bitmap_font_file_type::ebf, true},  supported_bitmap_font_file_type{".fnt", bitmap_font_file_type::fnt, false} };

		bool found{};
		supported_bitmap_font_file_type filetype{};
		for (const auto& extension : filetypes)
		{
			sprintf_s(buff, format_string.data(), base_path.data(), name.data(), extension.extension.data());

			if (filesystem::does_path_exist(buff))
			{
				filetype = extension;
				found = true;
				break;
			}

		}

		if (!found)
		{
			LOG_ERROR("File not found: {}", buff);
			return nullptr;
		}


		font::bitmap_font_resource_data resource_data{};
		auto file = filesystem::open(buff, file_mode::read, filetype.is_binary);

		switch (filetype.type)
		{
		case bitmap_font_file_type::fnt:
		{
			sprintf_s(buff, format_string.data(), base_path.data(), name.data(), ".ebf");

			std::string ebf_file_name = buff;
			resource_data = import_fnt_file(file, ebf_file_name);
			resource_data.data.type = font::type::bitmap;
		} break;
		case bitmap_font_file_type::ebf:
		{
			resource_data = read_ebf_file(file);
			resource_data.data.type = font::type::bitmap;
		} break;
		default:
		{
			LOG_WARN("Shouldn't have got here");
		}
		}

		resource_properties properties{ .type = resource_type::bitmap_font, .name = name.data(), .full_path = buff,};
		properties.data = malloc(sizeof(font::bitmap_font_resource_data));
		memcpy(properties.data, &resource_data, sizeof(font::bitmap_font_resource_data));
		return std::make_shared<resource>(properties);
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
			egkr::vector<uint8_t> line{};
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
				char buff[128];
				sscanf_s(line_string.data(), "info face=\"%[^\"]\" size=%u", buff, &data.data.size);
			} break;
			case 'c':
			{
				if (line[1] == 'o')
				{
					uint32_t num_pages{};
					sscanf_s(line_string.data(), "common lineHeight=%d base=%*d scaleW=%d scaleH=%d pages=%u", &data.data.line_height, &data.data.atlas_size_x, &data.data.atlas_size_y, &num_pages);

					data.pages.resize(num_pages);
				}
				else if (line[4] == 's')
				{
					uint32_t num_chars{};
					sscanf_s(line_string.data(), "chars count=%u", &num_chars);
				}
				else
				{
					font::glyph glyph{};
					sscanf_s(line_string.data(), "char id=%u x=%hu y=%hu width=%hu height=%hu xoffset=%hd yoffset=%hd xadvance=%hd page=%hhd", &glyph.codepoint, &glyph.x, &glyph.y, &glyph.width, &glyph.height, &glyph.x_offset, &glyph.y_offset, &glyph.x_advance, &glyph.page_id);
					data.data.glyphs.push_back(glyph);
				}
			} break;
			case 'p':
			{
				font::bitmap_font_page page{};
				char buff[128];
				sscanf_s(line_string.data(), "page id=%d file=\"%[^\"]\"", &page.id, buff);
				page.file = buff;
				data.pages.push_back(page);
			} break;
			case 'k':
			{
				if (line[7] == 's')
				{
					uint32_t kerning_count{};
					sscanf_s(line_string.data(), "kernings count=%d", &kerning_count);
				}
				else
				{
					font::kerning kerning{};
					sscanf_s(line_string.data(), "kerning first=%d second=%d amount=%hd", &kerning.codepoint_0, &kerning.codepoint_1, &kerning.amount);
					data.data.kernings.push_back(kerning);
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
		resource_header header{ };
		filesystem::read(handle, header);

		font::bitmap_font_resource_data data{};
		filesystem::read(handle, data.data.type);
		uint32_t face_length{};
		filesystem::read(handle, face_length);
		data.data.face.resize(face_length + 1);
		filesystem::read(handle, data.data.face.data(), sizeof(const char), data.data.face.size());
		filesystem::read(handle, data.data.size);
		filesystem::read(handle, data.data.line_height);
		filesystem::read(handle, data.data.baseline);
		filesystem::read(handle, data.data.atlas_size_x);
		filesystem::read(handle, data.data.atlas_size_y);

		uint32_t page_count{};
		filesystem::read(handle, page_count);
		for (auto i{0U}; i < page_count; ++i)
		{
			font::bitmap_font_page page{};
			filesystem::read(handle, page.id);

			uint32_t file_length{};
			filesystem::read(handle, file_length);
			page.file.resize(file_length + 1);
			filesystem::read(handle, page.file.data(), sizeof(const char), page.file.size());

			data.pages.push_back(page);
		}

		uint32_t glyph_count{};
		filesystem::write(handle, glyph_count);
		for (auto i{ 0U }; i < glyph_count; ++i)
		{
			font::glyph glyph{};
			filesystem::read(handle, glyph);
			data.data.glyphs.push_back(glyph);
		}

		uint32_t kerning_count{};
		filesystem::write(handle, kerning_count);
		if (kerning_count > 0)
		{
			for (auto i{ 0U }; i < kerning_count; ++i)
			{
				font::kerning kerning{};
				filesystem::read(handle, kerning);
			}
		}
		filesystem::read(handle, data.data.tab_advance);

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

		resource_header header{ .type = resource_type::bitmap_font };
		filesystem::write(handle, header, 1);

		filesystem::write(handle, data.data.type);
		filesystem::write(handle, data.data.face.size());
		filesystem::write(handle, data.data.face.data(), sizeof(const char), data.data.face.size());
		filesystem::write(handle, data.data.size);
		filesystem::write(handle, data.data.line_height);
		filesystem::write(handle, data.data.baseline);
		filesystem::write(handle, data.data.atlas_size_x);
		filesystem::write(handle, data.data.atlas_size_y);
		
		filesystem::write(handle, data.pages.size());
		for (const auto& page : data.pages)
		{
			filesystem::write(handle, page.id);
			filesystem::write(handle, page.file.size());
			filesystem::write(handle, page.file.data(), sizeof(const char), page.file.size());
		}

		filesystem::write(handle, data.data.glyphs.size());
		for (const auto& glyph : data.data.glyphs)
		{
			filesystem::write(handle, &glyph, sizeof(font::glyph), 1);
		}

		filesystem::write(handle, data.data.kernings.size());
		if (data.data.kernings.size() > 0)
		{
			for (const auto& kerning : data.data.kernings)
			{
				filesystem::write(handle, &kerning, sizeof(font::kerning), 1);
			}
		}
		filesystem::write(handle, data.data.tab_advance);

		return data;
	}
}