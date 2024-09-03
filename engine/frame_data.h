#pragma once

namespace egkr
{
	struct frame_data
	{
		float delta_time;
		double total_time;
		void* application_data{};
		uint64_t frame_number{};
		uint64_t draw_index{};
		uint32_t render_target_index{};
	};
}
