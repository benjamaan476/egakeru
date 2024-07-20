#pragma once

namespace egkr
{
	struct frame_data
	{
		float delta_time;
		double total_time;
		void* application_data{};
	};
}
