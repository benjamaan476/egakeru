#pragma once

#include <pch.h>
#include <resources/transform.h>
#include <interfaces/renderable.h>

#include <debug/debug_line.h>
#include <interfaces/transformable.h>

namespace egkr::debug
{
	class debug_frustum : public transformable, public renderable
	{
	public:
		using shared_ptr = std::shared_ptr<debug_frustum>;
		static shared_ptr create(const egkr::frustum& frustum);
		explicit debug_frustum(const egkr::frustum& frustum);

		void load();
		void unload();
		void update(const frustum& frustum);

		[[nodiscard]] const auto& get_id() const { return unique_id_; }

		void destroy();
	private:
		void recalculate_lines(const frustum& frustum);

	private:
		uint32_t unique_id_{ invalid_32_id };
		geometry::properties properties_{};
		egkr::vector<colour_vertex_3d> vertices_{};
	};
}
