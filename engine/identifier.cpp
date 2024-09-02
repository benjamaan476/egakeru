#include "identifier.h"

namespace egkr
{
	static egkr::vector<void*> owners;

	uint32_t identifier::acquire_unique_id(void* owner)
	{
		if (owners.empty())
		{
			owners.reserve(100);
			owners.push_back(nullptr);
		}

		for (auto i{ 0U }; i < owners.size(); ++i)
		{
			if (owners[i] == nullptr)
			{
				owners[i] = owner;
				return i;
			}
		}

		owners.push_back(owner);
		return (uint32_t)owners.size() - 1;
	}

	void identifier::release_id(uint32_t id)
	{
		if (id == invalid_32_id)
		{
			return;
		}

		if (owners.empty())
		{
			LOG_ERROR("Cannot release identifier Identifier system has not been used");
			return;
		}

		if (id > owners.size())
		{
			LOG_ERROR("Identifier exceeds registered amount");
				return;
		}
		owners[id] = nullptr;
	}
}
