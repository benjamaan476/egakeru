#pragma once

#include "pch.h"
#include <functional>
#include <variant>

namespace egkr
{
	enum class event_code : int16_t
	{
		quit = 1,
		key_up,
		key_down,
		mouse_up,
		mouse_down,
		mouse_move,
		resize,
		render_target_refresh_required,

		debug01,
		debug02,
		render_mode,
		hover_id_changed,
		event_code_size
	};

	template<class... Ts>
	struct overloaded : Ts...
	{
		using Ts::operator()...;
	};

	template<class... Ts>
	overloaded(Ts...) -> overloaded<Ts...>;

	struct event_context
	{
		using context = std::variant <
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

		context context_{};

		template<typename T>
		void get(uint32_t index, T& value) const
		{
			constexpr uint32_t count = 16 / sizeof(T);
			value = std::get<std::array<T, count>>(context_)[index];
		}
	};

	using event_callback = std::function<bool(event_code, void*, void*, const event_context&)>;

	class event
	{
	public:

		static void create();
		//bool on_event(event_code code, void* sender, void* listenter, const event_context& context);
		static bool register_event(event_code code, void* listener, const event_callback& callback);
		static bool unregister_event(event_code code, void* listener, const event_callback& callback);

		static void fire_event(event_code code, void* sender, const event_context& context);

	private:
		 
	};

	static event event_system;
}