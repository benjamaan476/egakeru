#pragma once
#include "pch.h"

#include "render_target.h"

namespace egkr
{
	class renderer_backend;
	namespace renderpass
	{
		enum clear_flags
		{
			none,
			colour = 0x01,
			depth = 0x02,
			stencil = 0x04,
			all = colour | depth | stencil
		};
		ENUM_CLASS_OPERATORS(clear_flags)

		struct configuration
		{
			std::string name{};
			float4 render_area{};
			float4 clear_colour{};
			clear_flags clear_flags{};
			float depth{};
			uint32_t stencil{};
			egkr::vector<render_target::render_target::configuration> targets{};
		};

		enum class state
		{
			ready,
			recording,
			in_render_pass,
			recording_ended,
			submitted,
			not_allocated
		};

		class renderpass
		{
		public:
			using shared_ptr = std::shared_ptr<renderpass>;
			renderpass(const renderer_backend* renderer, const configuration& configuration);
			virtual ~renderpass();

			virtual bool begin(render_target::render_target* render_target) const = 0;
			virtual bool end() = 0;
			virtual void free() = 0;

			[[nodiscard]] auto& get_render_targets() const {return render_targets;}
			[[nodiscard]] auto& get_render_target(uint32_t index) const {return render_targets[index];}
			void set_render_targets(const egkr::vector<render_target::render_target::shared_ptr>& targets)
			{
				render_targets = targets;
			}
			auto& get_render_area() { return render_area_; }
			void set_render_area(uint32_t width, uint32_t height);

		protected:
			float4 render_area_{};
			float4 clear_colour_{};
			clear_flags clear_flags_{};
			egkr::vector<render_target::render_target::shared_ptr> render_targets{ 3 };

		private:
			uint16_t id{ invalid_16_id };
		};
	}
}
