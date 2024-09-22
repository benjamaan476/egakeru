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
			entries.push_back({ .keymap_key = (key)i });
		}
	}

	void keymap::add_binding(key key, entry_bind_type type, modifier key_modifier, void* user_data, keybind_callback callback)
	{
		auto& binding_entry = entries[std::to_underlying(key)];
		auto* node = binding_entry.bindings;
		auto* previous = binding_entry.bindings;

		while (node)
		{
			previous = node;
			node = node->next;
		}

		binding* new_binding = new binding{ .type = type, .keymap_modifier = key_modifier, .callback = callback, .user_data = user_data };
		if (previous)
		{
			previous->next = new_binding;
		}
		else
		{
			binding_entry.bindings = new_binding;
		}
	}

	void keymap::remove_binding(key key, entry_bind_type type, modifier keymap_modifier, keybind_callback callback)
	{
		auto& binding_entry = entries[std::to_underlying(key)];

		auto* node = binding_entry.bindings;
		auto* previous = binding_entry.bindings;

		while (node)
		{
			if (node->callback.target_type() == callback.target_type() && node->keymap_modifier == keymap_modifier && node->type == type)
			{
				previous->next = node->next;
				delete node;
				return;
			}
		}
	}
}
