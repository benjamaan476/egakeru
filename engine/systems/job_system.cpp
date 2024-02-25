#include "job_system.h"

namespace egkr
{
	static job_system::unique_ptr state_{};

	job_system* job_system::create(const configuration& configuration)
	{
		state_ = std::make_unique<job_system>(configuration);
		return state_.get();
	}

	job_system::job_system(const configuration& configuration)
		: max_thread_count_{ 15 }, type_masks_{ configuration.type_masks }, thread_count_{ configuration.thread_count }
	{
		low_priority_queue_ = container::ring_queue<job::information>::create(1024, nullptr);
		normal_priority_queue_ = container::ring_queue<job::information>::create(1024, nullptr);
		high_priority_queue_ = container::ring_queue<job::information>::create(1024, nullptr);

		LOG_TRACE("Creating {} threads", thread_count_);
	}

	job_system::~job_system()
	{
		low_priority_queue_.reset();
		normal_priority_queue_.reset();
		high_priority_queue_.reset();
	}

	bool job_system::init()
	{
		if (thread_count_ > max_thread_count_)
		{
			LOG_ERROR("Exceeded the max thread count");
			return false;
		}
		running_ = true;

		for (auto i{ 0 }; i < thread_count_; ++i)
		{
			auto& thread = threads_[i];
			thread.index = i;
			thread.mask = type_masks_[i];
			thread.thread = std::jthread(job_system::run, &thread.index);
		}
		return true;
	}

	bool job_system::shutdown()
	{
		if (state_)
		{
			state_.reset();
			return true;
		}
		return false;
	}

	bool job_system::update(float /*delta_time*/)
	{
		if (!running_)
		{
			return false;
		}
		process_queue(high_priority_queue_.get(), &high_priority_queue_mutex_);
		process_queue(normal_priority_queue_.get(), &normal_priority_queue_mutex_);
		process_queue(low_priority_queue_.get(), &low_priority_queue_mutex_);

		for (int i{}; i < job::MAX_JOB_RESULTS; ++i)
		{
			job::result entry;
			{
				std::lock_guard lock{ results_mutex_ };
				entry = results_[i];
			}

			if (entry.index != invalid_16_id)
			{
				entry.callback(entry.params);

				if (entry.params)
				{
					free(entry.params);
				}
			}

			{
				std::lock_guard lock{ results_mutex_ };
				results_[i] = {};
			}
		}

		return true;
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


			for (auto i{ 0U }; i < thread_count; ++i)
			{
				auto& thread = state_->threads_[i];
				if ((uint32_t)(thread.mask & info.type) != 0)
				{
					bool found{};
					std::lock_guard lock{ thread.mutex };
					if (!thread.info.entry_point)
					{
						LOG_TRACE("Job immediately started");
						state_->threads_[i].info = info;
						found = true;
					}
					if (found)
					{
						return;
					}
				}
			}
		}

		if (info.priority == job::priority::low)
		{
			queue = state_->low_priority_queue_.get();
			mutex = &state_->low_priority_queue_mutex_;
		}

		std::lock_guard enqueue_lock{ *mutex };
		queue->enqueue(&info);
		LOG_TRACE("Job queued");

	}

	uint32_t job_system::run(void* params)
	{	
		uint32_t index = *(uint32_t*)params;
		auto& thread = state_->threads_[index];

		for (;;)
		{
			if (!state_ || !state_->running_)
			{
				break;
			}

			job::information info{};
			{
				std::lock_guard lock{ thread.mutex };
				info = thread.info;
			}

			if (info.entry_point)
			{
				bool result = info.entry_point(info.param_data, info.result_data);

				if (result && info.on_success)
				{
					store_result(info.on_success, info.result_data_size, info.result_data);
				}
				else if(!result && info.on_fail)
				{
					store_result(info.on_fail, info.result_data_size, info.result_data);
				}

				if (info.param_data)
				{
					free(info.param_data);
				}
				if (info.result_data)
				{
					free(info.result_data);
				}

				{
					std::lock_guard lock{ thread.mutex };
					thread.info = {};
				}

			}
			if (state_->running_)
			{
				std::this_thread::sleep_for(10ms);
			}
			else
			{
				break;
			}
		}

		return 1;
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

				{
					std::lock_guard lock{ thread.mutex };

					if (!thread.info.entry_point)
					{
						{
							std::lock_guard queue_lock{ *mutex };
							queue->dequeue(info);
						}
						thread.info = info;
						LOG_TRACE("Assigning job to thread");
						thread_found = true;
					}
				}
				if (thread_found)
				{
					break;
				}
			}
			if (!thread_found)
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
			memcpy(info.param_data, params, info.param_data_size);
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