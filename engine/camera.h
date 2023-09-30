#pragma once
#include "pch.h"

namespace egkr
{
	struct camera_properties
	{

	};

	class camera
	{
	public:
		using shared_ptr = std::shared_ptr<camera>;
		static shared_ptr create(const camera_properties& properties);

		explicit camera(const camera_properties& properties);
		~camera();

		destroy();

	private:

	};
}
