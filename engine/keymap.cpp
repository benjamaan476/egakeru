#include "keymap.h"

namespace egkr
{
	keymap keymap::create()
	{
		return {};
	}

	keymap::keymap()
	{
		for (auto i{0U}; i < std::to_underlying(key::key_count); ++i)
		{
			entries.push_back({ .keymap_key = (key)i });
		}
	}

	void keymap::add_binding(key key, entry_bind_type type, modifier key_modifier, void* user_data, keybind_callback callback)
	{
		auto& binding_entry = entries[std::to_underlying(key)];
		auto& node = binding_entry.bindings;

		node.emplace_back(type, key_modifier, callback, user_data);
	}

	void keymap::remove_binding(key key, entry_bind_type type, modifier keymap_modifier, keybind_callback callback)
	{
		auto& binding_entry = entries[std::to_underlying(key)];

		for(auto it = binding_entry.bindings.begin(); it != binding_entry.bindings.end(); it++)
		{
			if (it->callback.target_type() == callback.target_type() && it->keymap_modifier == keymap_modifier && it->type == type)
			{
				binding_entry.bindings.erase(it);
				return;
			}
		}
	}
}
