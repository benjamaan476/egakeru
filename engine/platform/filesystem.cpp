#include "filesystem.h"

#include <filesystem>

namespace egkr
{
	bool filesystem::does_path_exist(std::string_view path)
	{
		const std::filesystem::path filepath{ path };
		const auto absolute = std::filesystem::absolute(path);
		return std::filesystem::exists(absolute);
	}
	file_handle filesystem::open(std::string_view path, file_mode mode, bool is_binary)
	{
		auto absolute_filepath = std::filesystem::absolute(path);
		if (!does_path_exist(path) && (mode & file_mode::read))
		{
			LOG_WARN("Invalid file path, does not exist: {}", path.data());
			return { {}, "", false };
		}


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
		FILE* handle{};
		fopen_s(&handle, absolute_filepath.string().c_str(), openmode.c_str());

		if (!handle)
		{
			LOG_WARN("Failed to open file: {}", path.data());
			return { {}, "", false };
		}

		return { handle, absolute_filepath.string(), true };
	}

	void filesystem::close(file_handle& handle)
	{
		if (handle.handle)
		{
			fclose(handle.handle);
		}
	}

	egkr::vector<uint8_t> filesystem::read_line(file_handle& handle, int32_t max_size)
	{
		if (!handle.is_valid)
		{
			LOG_WARN("Invalid file handle passed");
			return {};
		}

		char buff[max_size];
		auto ret = fgets(buff, max_size, handle.handle);

		if (ret == nullptr)
		{
			return {};
		}
		buff[strcspn(buff, "\n")] = 0;
		auto len = strlen(buff);
		if (len == 0)
		{
			return { '\0' };
		}
		return std::vector<uint8_t>(buff, buff + len);
	}

	uint64_t filesystem::write_line(file_handle& handle, const egkr::vector<uint8_t>& line)
	{
		if (!handle.is_valid)
		{
			LOG_WARN("Invalid file handle passed");
			return {};
		}

		auto result = fputs((const char*)line.data(), handle.handle);
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


	egkr::vector<uint8_t> filesystem::read_all(file_handle& handle)
	{
		if (!handle.is_valid)
		{
			LOG_WARN("Invalid file handle passed");
			return {};
		}

		auto size = (size_t)std::filesystem::file_size(handle.filepath);

		egkr::vector<uint8_t> data(size);
		auto count = 0; 
		size_t read = 0;
		while ((read = fread(data.data() + count, 1, size, handle.handle)) > 0)
		{
			count += read;
		}
		return data;
	}
}
