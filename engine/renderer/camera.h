#pragma once
#include "pch.h"

namespace egkr
{
	class camera
	{
	public:
		using shared_ptr = std::shared_ptr<camera>;
		static shared_ptr create(std::string_view name);

		explicit camera(std::string_view name);

		void reset();
		[[nodiscard]] const auto& get_position() const { return position_;}
		void set_position(const float3& pos);

		[[nodiscard]] const auto& get_rotation() const { return rotation_;}
		void set_rotation(const float3& rot);

		[[nodiscard]] float get_aspect() const { return aspect_ratio_; }
		void set_aspect(float aspect);

		[[nodiscard]] float4x4 get_view();
		[[nodiscard]] float4x4 get_projection() const;

		[[nodiscard]] float3 get_forward() const;
		[[nodiscard]] float3 get_back() const;
		[[nodiscard]] float3 get_left() const;
		[[nodiscard]] float3 get_right() const;
		[[nodiscard]] float3 get_up() const;
		[[nodiscard]] float3 get_down() const;

		[[nodiscard]] float get_fov() const;
		[[nodiscard]] float get_far_clip() const;
		[[nodiscard]] float get_near_clip() const;

		void move_forward(float_t amount);
		void move_back(float_t amount);
		void move_left(float_t amount);
		void move_right(float_t amount);
		void move_up(float_t amount);
		void move_down(float_t amount);

		void yaw(float amount);
		void pitch(float amount);

	private:
		std::string name_;
		float3 position_{};
		float3 rotation_{};
		bool is_dirty_{true};

		float4x4 view_{};
		
		float fov_{glm::radians(45.F)};
		float near_clip_{0.1F};
		float far_clip_{ 1000.F };
		float aspect_ratio_{ 1.333f };
	};
}
