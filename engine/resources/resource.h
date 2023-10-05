#pragma once
#include "pch.h"

namespace egkr
{
	template<class T>
	class resource
	{
	public:
		using shared_ptr = std::shared_ptr<T>;

		resource() = default;
		resource(uint32_t id, uint32_t generation) : id_{ id }, generation_{ generation } {}

		const auto& get_id() const { return id_; }
		void set_id(uint32_t id) { id_ = id; }

		const auto& get_generation() const { return generation_; }
		void increment_generation() { ++generation_; }
		void set_generation(uint32_t generation) { generation_ = generation; }
	private:
		uint32_t id_{ invalid_id };
		uint32_t generation_{ invalid_id };
	};
}
