#pragma once
#include <pch.h>
#include <span>

#include <containers/ring_queue.h>
#include <resources/job.h>

#include <systems/system.h>

namespace egkr
{

	class job_system : public system
	{
	public:
		struct configuration
		{
			uint8_t thread_count{};
			egkr::vector<job::type> type_masks;
		};

		using unique_ptr = std::unique_ptr<job_system>;
		static job_system* create(const configuration& configuration);

		explicit job_system(const configuration& configuration);
		~job_system() override;

		bool init() override;
		bool update(const frame_data& frame_data) override;
		bool shutdown() override;

		static void submit(job::information info);
		static uint32_t run(void* params);
		static void process_queue(container::ring_queue<job::information>* queue, std::mutex* mutex);

		static job::information create_job(job::start_job entry_point, job::complete_job on_success, job::complete_job on_fail, void* params, uint32_t params_size, uint32_t result_size);
		static job::information create_job(job::start_job entry_point, job::complete_job on_success, job::complete_job on_fail, job::type type, void* params, uint32_t params_size, uint32_t result_size);
		static job::information create_job(job::start_job entry_point, job::complete_job on_success, job::complete_job on_fail, job::type type, job::priority priority, void* params, uint32_t params_size, uint32_t result_size);

	private:
		static void store_result(job::complete_job on_complete, uint32_t param_size, void* params);
	private:
		bool running_{};
		uint8_t max_thread_count_{};
		egkr::vector<job::type> type_masks_;

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
