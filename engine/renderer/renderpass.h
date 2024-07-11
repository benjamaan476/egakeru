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
			render_target::configuration target{};
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
			static shared_ptr create(const configuration& configuration);
			explicit renderpass(const configuration& configuration);
			virtual ~renderpass();

			virtual bool begin(render_target::render_target* render_target) const = 0;
			virtual bool end() const = 0;
			virtual void free() = 0;

			uint32_t get_render_target_count() const { return (uint32_t)render_targets_.size(); }
			[[nodiscard]] auto& get_render_targets() {return render_targets_;}
			[[nodiscard]] const auto& get_render_target(uint32_t index) const {return render_targets_[index];}
			[[nodiscard]] auto& get_render_target(uint32_t index) {return render_targets_[index];}
			void set_render_targets(const egkr::vector<render_target::render_target::shared_ptr>& targets)
			{
				render_targets_ = targets;
			}
			auto& get_render_area() { return render_area_; }
			void set_render_area(uint32_t width, uint32_t height);

		protected:
			float4 render_area_{};
			float4 clear_colour_{};
			clear_flags clear_flags_{};
			float_t depth_{};
			uint32_t stencil_{};
			egkr::vector<render_target::render_target::shared_ptr> render_targets_{ 3 };

		private:
			uint16_t id{ invalid_16_id };
		};
	}
}
