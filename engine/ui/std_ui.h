#pragma once
#include "pch.h"

#include "resources/transform.h"

namespace egkr::sui
{
	struct control : public transformable
	{
		std::string name;
		uint32_t unique_id{ invalid_32_id };

		std::list<control> children;

		using shared_ptr = std::shared_ptr<control>;
		static shared_ptr create();

		bool destroy();
		bool load();
		bool unload();
		bool update(const frame_data& frame_data);
		bool render(const frame_data& frame_data);

		bool is_active() const { return is_active_; }
		bool is_visible() const { return is_visible_; }

	private:
		bool is_active_{ true };
		bool is_visible_{ true };
	};
}
