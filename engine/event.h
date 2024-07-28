#pragma once

#include "pch.h"
#include <functional>
#include <variant>

namespace egkr
{
	class event
	{
	public:
		enum class code : int16_t
		{
			quit = 1,
			key_up,
			key_down,
			mouse_up,
			mouse_down,
			mouse_move,
			mouse_wheel,
			resize,
			render_target_refresh_required,
			evar_changed,

			debug01,
			debug02,
			debug03,
			render_mode,
			hover_id_changed,
			event_code_size
		};

		struct context
		{
			using ctx = std::variant <
				std::array<int64_t, 2>,
				std::array<uint64_t, 2>,
				std::array<double_t, 2>,

				std::array<float_t, 4>,
				std::array<int32_t, 4>,
				std::array<uint32_t, 4>,

				std::array<int16_t, 8>,
				std::array<uint16_t, 8>,

				std::array<int8_t, 16>,
				std::array<uint8_t, 16>>;


			template<typename T>
			void get(uint32_t index, T& value) const
			{
				constexpr uint32_t count = 16 / sizeof(T);
				value = std::get<std::array<T, count>>(context_)[index];
			}

			template<typename T>
			void set(uint32_t index, T value)
			{
				constexpr uint32_t count = 16 / sizeof(T);
				if (std::holds_alternative<std::array<T, count>>(context_))
				{
					auto& v = std::get<std::array<T, count>>(context_);
					v[index] = value;
				}
				else
				{
					auto array = std::array<T, count>{};
					array[index] = value;
					context_.emplace<std::array<T, count>>(array);
				}
				//context_ = v;
			}

		private:
			ctx context_{};
		};

		using callback = std::function<bool(code, void*, void*, const context&)>;

		static void create();
		static bool register_event(code code, void* listener, const callback& callback);
		static bool unregister_event(code code, void* listener, const callback& callback);
		static void fire_event(code code, void* sender, const context& context);
	};
}
