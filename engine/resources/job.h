#pragma once
#include <pch.h>
#include <thread>
#include <mutex>

namespace egkr
{
	namespace job
	{
		using start_job = std::function<bool(void*, void*)>;
		using complete_job = std::function<void(void*)>;

		enum class type
		{
			general = 0x02,
			resource_load = 0x04,
			gpu_resource = 0x08
		};
		ENUM_CLASS_OPERATORS(type)

		enum class priority
		{
			low,
			medium,
			high
		};

		struct information
		{
			start_job entry_point{};
			complete_job on_success{};
			complete_job on_fail{};

			type type;
			priority priority{ priority::medium };
			void* param_data{};
			uint32_t param_data_size{};

			void* result_data{};
			uint32_t result_data_size{};

			//information() = default;
			//information(const information& info) = default;

			~information()
			{}
		};

		struct thread
		{
			uint8_t index{};
			std::jthread thread{};
			information info{};
			std::mutex mutex{};
			type mask{};
		};

		struct result
		{
			uint16_t index{invalid_16_id};
			complete_job callback{};
			uint32_t param_size{};
			void* params{};
		};

		constexpr static int32_t MAX_JOB_RESULTS = 512;

		class job
		{
		public:
		private:

		};
	}
}