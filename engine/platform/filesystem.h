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
		static uint64_t write(file_handle& handle, const egkr::vector<uint8_t>& data);

		static egkr::vector<uint8_t> read_all(file_handle& handle);
	};
}
