#include "job_system.h"

namespace egkr::job_system
{
	static job_system::unique_ptr state_{};

	bool job_system::create(const configuration& configuration)
	{
		state_ = std::make_unique<job_system>(configuration);
		return true;
	}

	job_system::job_system(const configuration& configuration)
		: max_thread_count_{ 15 }, type_masks_{ configuration.type_masks }, thread_count_{ configuration.thread_count }
	{
		low_priority_queue_ = container::ring_queue<job::information>::create(1024, nullptr);
		normal_priority_queue_ = container::ring_queue<job::information>::create(1024, nullptr);
		high_priority_queue_ = container::ring_queue<job::information>::create(1024, nullptr);

		LOG_TRACE("Creating {} threads", thread_count_);
	}

	bool job_system::init()
	{
		state_->running_ = true;

		for (auto i{ 0 }; i < state_->thread_count_; ++i)
		{
			auto& thread = state_->threads_[i];
			thread.index = i;
			thread.mask = state_->type_masks_[i];
			thread.thread = std::jthread(job_system::run, &thread.index);
		}
		return true;
	}

	bool job_system::shutdown()
	{
		if (state_)
		{
			state_->low_priority_queue_.reset();
			state_->normal_priority_queue_.reset();
			state_->high_priority_queue_.reset();
			state_ = nullptr;
			return true;
		}
		return false;
	}

	void job_system::update()
	{
		if (!state_->running_)
		{
			return;
		}
		process_queue(state_->high_priority_queue_.get(), &state_->high_priority_queue_mutex_);
		process_queue(state_->normal_priority_queue_.get(), &state_->normal_priority_queue_mutex_);
		process_queue(state_->low_priority_queue_.get(), &state_->low_priority_queue_mutex_);

		for (int i{}; i < job::MAX_JOB_RESULTS; ++i)
		{
			std::lock_guard lock{ state_->results_mutex_ };
			auto& entry = state_->results_[i];

			if (entry.index == invalid_16_id)
			{
				entry.callback(entry.params);

				if (entry.params)
				{
					free(entry.params);
				}
			}

			entry = job::result{.index = invalid_16_id};
		}
	}

	void job_system::submit(job::information info)
	{
		uint64_t thread_count = state_->thread_count_;
		container::ring_queue<job::information>* queue = state_->normal_priority_queue_.get();
		std::mutex* mutex = &state_->normal_priority_queue_mutex_;

		if (info.priority == job::priority::high)
		{
			queue = state_->high_priority_queue_.get();
			mutex = &state_->high_priority_queue_mutex_;
		}

		for (auto i{ 0U}; i < thread_count; ++i)
		{
			auto& thread = state_->threads_[i];
			if ((uint32_t)(thread.mask & info.type) != 0)
			{
				std::lock_guard lock{ thread.mutex };
				if (thread.info.entry_point)
				{

				}
				thread.info = info;
			}
		}

		if (info.priority == job::priority::low)
		{
			queue = state_->low_priority_queue_.get();
			mutex = &state_->low_priority_queue_mutex_;
		}

		std::lock_guard enqueue_lock{ *mutex };
		queue->enqueue(&info);


	}

	uint32_t job_system::run(void* params)
	{	
		uint32_t index = *(uint32_t*)params;
		auto& job = state_->threads_[index];

		for (;;)
		{
			if (!state_->running_)
			{
				break;
			}

			job::information info{};
			{
				std::lock_guard lock{ job.mutex };
				info = job.info;
			}

			if (info.entry_point)
			{
				bool result = info.entry_point(info.param_data, info.result_data);

				if (result && info.on_success)
				{
					store_result(info.on_success, info.result_data_size, info.result_data);
				}
				else
				{
					store_result(info.on_fail, info.result_data_size, info.result_data);
				}

				if (info.param_data)
				{
					free(info.param_data);
				}

				{
					std::lock_guard lock{ job.mutex };
					info.result_data = {};
					job.info = {};
				}

				if (state_->running_)
				{
					std::this_thread::sleep_for(10ms);
				}
			}
			else
			{
				break;
			}
		}

		return 0;
	}

	void job_system::process_queue(container::ring_queue<job::information>* queue, std::mutex* mutex)
	{
		uint64_t thread_count = state_->thread_count_;
		while (queue->get_length() > 0)
		{
			job::information info{};

			if (!queue->peek(info))
			{
				LOG_ERROR("Bad queue");
				return;
			}
			bool thread_found{};

			for (auto i{ 0U }; i < thread_count; ++i)
			{
				job::thread& thread = state_->threads_[i];
				if ((uint32_t)(thread.mask & info.type) == 0)
				{
					continue;
				}

				std::lock_guard lock{ thread.mutex };

				if (!info.entry_point)
				{
					std::lock_guard queue_lock{ *mutex };
					queue->dequeue(info);
				}
				thread.info = info;
				thread_found = true;
			}

			if (thread_found)
			{
				break;
			}
		}
	}

	job::information job_system::create_job(job::start_job entry_point, job::complete_job on_success, job::complete_job on_fail, void* params, uint32_t param_size, uint32_t result_size)
	{
		return create_job(entry_point, on_success, on_fail, job::type::general, params, param_size, result_size);
	}

	job::information job_system::create_job(job::start_job entry_point, job::complete_job on_success, job::complete_job on_fail, job::type type, void* params, uint32_t param_size, uint32_t result_size)
	{
		return create_job(entry_point, on_success, on_fail, type, job::priority::medium, params, param_size, result_size);
	}

	job::information job_system::create_job(job::start_job entry_point, job::complete_job on_success, job::complete_job on_fail, job::type type, job::priority priority, void* params, uint32_t param_size, uint32_t result_size)
	{
		job::information info{ .entry_point = entry_point, .on_success = on_success, .on_fail = on_fail, .type = type, .priority = priority, .param_data_size = param_size };

		if (param_size)
		{
			info.param_data = malloc(param_size);
		}

		if (result_size)
		{
			info.result_data_size = result_size;
			info.result_data = malloc(result_size);
		}
		return info;
	}

	void job_system::store_result(job::complete_job on_complete, uint32_t param_size, void* params)
	{
		job::result result{ .index = invalid_16_id, .callback = on_complete, .param_size = param_size };

		if (param_size > 0)
		{
			result.params = malloc(param_size);
			memcpy(result.params, params, param_size);
		}

		{
			std::lock_guard lock{ state_->results_mutex_ };
			for (int i{}; i < job::MAX_JOB_RESULTS; ++i)
			{
				if (state_->results_[i].index == invalid_16_id)
				{
					state_->results_[i] = result;
					state_->results_[i].index = i;
					break;
				}
			}
		}
	}
}