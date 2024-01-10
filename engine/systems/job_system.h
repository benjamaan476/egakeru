#pragma once
#include <pch.h>
#include <span>

#include <containers/ring_queue.h>
#include <resources/job.h>

namespace egkr
{
	namespace job_system
	{
		struct configuration
		{
			uint8_t thread_count{};
			egkr::vector<uint32_t> type_masks;
		};

		class job_system
		{
		public:
			using unique_ptr = std::unique_ptr<job_system>;
			static bool create(const configuration& configuration);

			explicit job_system(const configuration& configuration);

			static bool init();
			static bool shutdown();

			static void update();
			static void submit(job::information info);

			job::information create_job(job::start_job entry_point, job::complete_job on_success, job::complete_job on_fail, std::span<void*> params, std::span<void*> result);
			job::information create_job(job::start_job entry_point, job::complete_job on_success, job::complete_job on_fail, job::type type, std::span<void*> params, std::span<void*> result);
			job::information create_job(job::start_job entry_point, job::complete_job on_success, job::complete_job on_fail, job::type type, job::priority priority, std::span<void*> params, std::span<void*> result);
			
		private:
			bool running_{};
			uint8_t max_thread_count_{};
			egkr::vector<uint32_t> type_masks_;

			uint8_t thread_count_{};
			std::array<job::thread, 32> threads_{};

			container::ring_queue<job::information>::unique_ptr low_priority_queue_;
			container::ring_queue<job::information>::unique_ptr normal_priority_queue_;
			container::ring_queue<job::information>::unique_ptr high_priority_queue_;

			std::mutex low_priority_queue_mutex_{};
			std::mutex normal_priority_queue_mutex_{};
			std::mutex high_priority_queue_mutex_{};

			std::array<job::result, job::MAX_JOB_RESULTS> results_{};
			std::mutex results_mutex_{};
		};
	}
}