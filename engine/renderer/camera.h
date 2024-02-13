#pragma once
#include "pch.h"

namespace egkr
{
	class camera
	{
	public:
		using shared_ptr = std::shared_ptr<camera>;
		static shared_ptr create(std::string_view name);

		camera(std::string_view name);

		void reset();
		[[nodiscard]] const auto& get_position() const { return position_;}
		void set_position(const float3& pos);

		[[nodiscard]] const auto& get_rotation() const { return rotation_;}
		void set_rotation(const float3& rot);

		[[nodiscard]] float4x4 get_view();

		[[nodiscard]] float3 get_forward() const;
		[[nodiscard]] float3 get_back() const;
		[[nodiscard]] float3 get_left() const;
		[[nodiscard]] float3 get_right() const;
		[[nodiscard]] float3 get_up() const;
		[[nodiscard]] float3 get_down() const;

		void move_forward(float_t amount);
		void move_back(float_t amount);
		void move_left(float_t amount);
		void move_right(float_t amount);
		void move_up(float_t amount);
		void move_down(float_t amount);

		void camera_yaw(float amount);
		void camera_pitch(float amount);

	private:
		std::string name_{};
		float3 position_{};
		float3 rotation_{};
		bool is_dirty_{true};

		float4x4 view_{};
	};
}
