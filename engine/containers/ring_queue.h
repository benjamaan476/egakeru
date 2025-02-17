#pragma once
#include <pch.h>

namespace egkr::container
{
	template<class T>
	class ring_queue
	{
	public:
		using unique_ptr = std::unique_ptr<ring_queue<T>>;
		static unique_ptr create(uint32_t capacity, T* memory);

		ring_queue(uint32_t capacity, T* memory);
		~ring_queue();

		static void destroy();

		bool enqueue(T* value);
		bool dequeue(T& value);
		bool peek(T& value);

		[[nodiscard]] const auto& get_length() const { return length_;}

	private:
		const uint32_t stride_{ sizeof(T) };
		uint32_t length_{};
		uint32_t capacity_{};

		std::vector<T> block_{};
		bool owns_memory_{};

		uint32_t head_{};
		uint32_t tail_{ invalid_32_id };
	};

	template<class T>
	inline container::ring_queue<T>::unique_ptr ring_queue<T>::create(uint32_t capacity, T* memory)
	{
		return std::make_unique<ring_queue<T>>(capacity, memory);
	}

	template<class T>
	inline ring_queue<T>::ring_queue(uint32_t capacity, T* memory)
		: capacity_{ capacity }
	{
		if (memory)
		{
			owns_memory_ = false;
		}
		else
		{
			owns_memory_ = true;
			block_.resize(capacity);
		}
	}

	template<class T>
	inline ring_queue<T>::~ring_queue()
	{
		if (owns_memory_)
		{
			block_.clear();
		}
	}

	template<class T>
	inline bool ring_queue<T>::enqueue(T* value)
	{
		if (length_ == capacity_)
		{
			LOG_ERROR("Capacity exceeded");
			return false;
		}

		tail_ += 1;
		tail_ %= capacity_;
		
		block_[tail_] = *value;
		++length_;
		return true;
	}

	template<class T>
	inline bool ring_queue<T>::dequeue(T& value)
	{
		if (length_ == 0)
		{
			LOG_ERROR("Can't dequeue empty ring_queue");
			return false;
		}

		value = block_[head_];
		head_ += 1;
		head_ %= capacity_;

		--length_;
		return true;
	}

	template<class T>
	inline bool ring_queue<T>::peek(T& value)
	{
		if (length_ == 0)
		{
			LOG_ERROR("Can't dequeue empty ring_queue");
			return false;
		}

		value = block_[head_];
		return true;
	}
}
