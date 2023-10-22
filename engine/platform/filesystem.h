#pragma once
#include "pch.h"

#include <fstream>

namespace egkr
{
	struct file_handle
	{
		FILE* handle{};
		std::string filepath{};
		bool is_valid{};

		~file_handle()
		{
			if (handle)
			{
				fclose(handle);
			}
		}
	};

	enum file_mode { read = 1, write = 2 };

	class filesystem
	{
	public:
		static bool does_path_exist(std::string_view path);
		[[nodiscard]] static file_handle open(std::string_view path, file_mode mode, bool is_binary);
		static void close(file_handle& handle);

		static egkr::vector<uint8_t> read_line(file_handle& handle, uint32_t max_size);
		static uint64_t write_line(file_handle& handle, const egkr::vector<uint8_t>& line);

		// Returns the read data and size of read data
		static egkr::vector<uint8_t> read(file_handle& handle, uint64_t size);

		template<class T>
		static void read(file_handle& handle, T* data, uint32_t count);

		template<class T>
		static void read(file_handle& handle, T* data, uint32_t size, uint32_t count);

		template<class type>
		static uint64_t write(file_handle& handle, const egkr::vector<type>& data);

		template<class type>
		static uint64_t write(file_handle& handle, const type& data, uint32_t count);
		template<class type>
		static uint64_t write(file_handle& handle, type* data, uint32_t size, uint32_t count);

		static egkr::vector<uint8_t> read_all(file_handle& handle);
	};

	template<class T>
	inline void filesystem::read(file_handle& handle, T* data, uint32_t count)
	{
		if (!handle.is_valid)
		{
			LOG_WARN("Invalid file handle passed");
			return;
		}

		auto read_bytes = fread(data, sizeof(T), count, handle.handle);
		if (read_bytes != count)
		{
			LOG_WARN("Read bytes does not match specified size: {}, {}", read_bytes, sizeof(T) * count);
		}
	}

	template<class T>
	inline void filesystem::read(file_handle& handle, T* data, uint32_t size, uint32_t count)
	{
		if (!handle.is_valid)
		{
			LOG_WARN("Invalid file handle passed");
			return;
		}

		auto read_bytes = fread(data, size, count, handle.handle);
		if (read_bytes != count)
		{
			LOG_WARN("Read bytes does not match specified size: {}, {}", read_bytes, size * count);
		}

	}

	template<class type>
	inline uint64_t filesystem::write(file_handle& handle, const egkr::vector<type>& data)
	{
		if (!handle.is_valid)
		{
			LOG_WARN("Invalid file handle passed");
			return {};
		}

		auto wrote_bytes = fwrite(data.data(), sizeof(type), data.size(), handle.handle);

		if (wrote_bytes != data.size())
		{
			LOG_WARN("Written byte size does not match data size");
		}

		return data.size();
	}

	template<class type>
	inline uint64_t filesystem::write(file_handle& handle, const type& data, uint32_t count)
	{
		const auto size = sizeof(type);
		if (!handle.is_valid)
		{
			LOG_WARN("Invalid file handle passed");
			return {};
		}

		auto wrote_bytes = fwrite(&data, size, count, handle.handle);

		if (wrote_bytes != count)
		{
			LOG_WARN("Written byte size does not match data size");
		}

		return wrote_bytes;
	}
	template<class type>
	inline uint64_t filesystem::write(file_handle& handle, type* data, uint32_t size, uint32_t count)
	{
		if (!handle.is_valid)
		{
			LOG_WARN("Invalid file handle passed");
			return {};
		}

		auto wrote_bytes = fwrite(data, size, count, handle.handle);

		if (wrote_bytes != count)
		{
			LOG_WARN("Written byte size does not match data size");
		}

		return wrote_bytes;
	}
}
