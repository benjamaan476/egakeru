#include "keymap.h"

namespace egkr
{
	keymap keymap::create()
	{
		return keymap();
	}

	keymap::keymap()
	{
		for (auto i{0U}; i < std::to_underlying(key::key_count); ++i)
		{
			entries.push_back({ .key = (key)i });
		}
	}

	void keymap::add_binding(key key, entry_bind_type type, modifier modifier, void* user_data, keybind_callback callback)
	{
		auto& entry = entries[std::to_underlying(key)];
		auto* node = entry.bindings;
		auto* previous = entry.bindings;

		while (node)
		{
			previous = node;
			node = node->next;
		}

		binding* new_binding = new binding{ .type = type, .modifier = modifier, .callback = callback, .user_data = user_data };
		if (previous)
		{
			previous->next = new_binding;
		}
		else
		{
			entry.bindings = new_binding;
		}
	}

	void keymap::remove_binding(key key, entry_bind_type type, modifier modifier, keybind_callback callback)
	{
		auto& entry = entries[std::to_underlying(key)];

		auto* node = entry.bindings;
		auto* previous = entry.bindings;

		while (node)
		{
			if (node->callback.target_type() == callback.target_type() && node->modifier == modifier && node->type == type)
			{
				previous->next = node->next;
				delete node;
				return;
			}
		}
	}
}
