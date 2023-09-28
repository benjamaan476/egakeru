#include "filesystem.h"

#include <filesystem>

namespace egkr
{
	bool filesystem::does_path_exist(std::string_view path)
	{
		std::filesystem::path filepath{ path };
		auto absolute = std::filesystem::absolute(path);
		return std::filesystem::exists(absolute);
	}
	file_handle filesystem::open(std::string_view path, file_mode mode, bool is_binary)
	{
		if (!does_path_exist(path))
		{
			LOG_WARN("Invalid file path, does not exist: {}", path.data());
			return { {}, "", false };
		}

		auto absolute_filepath = std::filesystem::absolute(path);

		std::string openmode{};

		if ((mode & file_mode::read) != 0 && (mode & file_mode::write) != 0)
		{
			openmode = is_binary ? "w+b" : "w+";
		}
		else if ((mode & file_mode::read) != 0 && (mode & file_mode::write) == 0)
		{
			openmode = is_binary ? "rb" : "r";
		}
		else if ((mode & file_mode::read) == 0 && (mode & file_mode::write) != 0)
		{
			openmode = is_binary ? "wb" : "w";
		}
		else
		{
			LOG_ERROR("Invalid mode passed while trying to open file: '%s'", path);
			return { {}, "", false };
		}
		FILE* f{};
		fopen_s(&f, absolute_filepath.string().c_str(), openmode.c_str());

		if (!f)
		{
			LOG_WARN("Failed to open file: {}", path.data());
			return { {}, "", false };
		}

		return { f, absolute_filepath.string(), true };
	}

	void filesystem::close(file_handle& handle)
	{
		if (handle.handle)
		{
			fclose(handle.handle);
		}
	}

	egkr::vector<uint8_t> filesystem::read_line(file_handle& handle, uint32_t max_size)
	{
		if (!handle.is_valid)
		{
			LOG_WARN("Invalid file handle passed");
			return {};
		}

		egkr::vector<uint8_t> data(max_size);
		fgets((char*)data.data(), data.size(), handle.handle);

		return data;
	}

	uint64_t filesystem::write_line(file_handle& handle, const egkr::vector<uint8_t>& line)
	{
		if (!handle.is_valid)
		{
			LOG_WARN("Invalid file handle passed");
			return {};
		}

		auto result = fputs((char*)line.data(), handle.handle);
		if (result != EOF)
		{
			fputs("\n", handle.handle);
		}

		fflush(handle.handle);
		return 0;
	}

	egkr::vector<uint8_t> filesystem::read(file_handle& handle, uint64_t size)
	{
		if (!handle.is_valid)
		{
			LOG_WARN("Invalid file handle passed");
			return {};
		}

		egkr::vector<uint8_t> data(size);
		auto read_bytes = fread(data.data(), 1, size, handle.handle);
		if (read_bytes != size)
		{
			LOG_WARN("Read bytes does not match specified size: {}, {}", read_bytes, size);
		}

		return data;
	}

	uint64_t filesystem::write(file_handle& handle, const egkr::vector<uint8_t>& data)
	{
		if (!handle.is_valid)
		{
			LOG_WARN("Invalid file handle passed");
			return {};
		}

		auto wrote_bytes = fwrite(data.data(), 1, data.size(), handle.handle);

		if (wrote_bytes != data.size())
		{
			LOG_WARN("Written byte size does not match data size");
		}

		return data.size();
	}

	egkr::vector<uint8_t> filesystem::read_all(file_handle& handle)
	{
		if (!handle.is_valid)
		{
			LOG_WARN("Invalid file handle passed");
			return {};
		}

		auto size = std::filesystem::file_size(handle.filepath);

		egkr::vector<uint8_t> data(size);
		fread(data.data(), 1, size, handle.handle);
		return data;
	}
}