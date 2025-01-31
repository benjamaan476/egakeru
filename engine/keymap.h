#pragma once
#include "pch.h"
#include "keys.h"
#include <list>

namespace egkr
{
	class keymap
	{
	public:
		enum modifier : uint8_t
		{
			none = 0x0,
			shift = 0x1,
			control = 0x2,
			alt = 0x4
		};

		enum entry_bind_type : uint8_t
		{
			undefined = 0x0,
			press = 0x1,
			release = 0x2,
			hold = 0x4,
			unset = 0x8
		};

		using keybind_callback = std::function<void(key, entry_bind_type, modifier, void*)>;

		struct binding
		{
			entry_bind_type type{};
			modifier keymap_modifier{};
			keybind_callback callback;
			void* user_data{};
		};

		struct entry
		{
			key keymap_key{};
			std::list<binding> bindings;
		};

		static keymap create();

		keymap();
		void add_binding(key key, entry_bind_type type, modifier modifier, void* user_data, keybind_callback callback);
		void remove_binding(key key, entry_bind_type type, modifier modifier, keybind_callback callback);

		bool overrides_all{};
		egkr::vector<entry> entries;
	};
}
