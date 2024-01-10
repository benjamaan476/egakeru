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
			thread.thread = std::jthread(job::job::run, &thread.index);
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
}