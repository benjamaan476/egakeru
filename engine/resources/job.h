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

		enum class priority
		{
			low,
			medium,
			high
		};

		struct information
		{
			type type;
			priority priority{ priority::medium };
			start_job entry_point{};
			complete_job on_success{};
			complete_job on_fail{};

			void* param_data{};
			uint32_t param_data_size{};

			void* result_data{};
			uint32_t result_data_size{};
		};

		struct thread
		{
			uint8_t index{};
			std::jthread thread{};
			information info{};
			std::mutex mutex{};
			uint32_t mask{};

		};

		struct result
		{
			uint16_t index{};
			complete_job callback{};
			uint32_t param_size{};
			void* params{};
		};

		constexpr static int32_t MAX_JOB_RESULTS = 512;

		class job
		{
		public:
			static uint32_t run(void* params);
		private:

		};
	}
}