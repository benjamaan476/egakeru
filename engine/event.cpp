#include "event.h"

namespace egkr
{
	struct registered_event
	{
		void* listener{};
		event::callback callback;

		bool operator==(const registered_event& other) const
		{
			return listener == other.listener;
		}
	};

	struct event_code_entry
	{
		std::vector<registered_event> event;
	};

	struct event_system_state
	{
		std::array<event_code_entry, (size_t)event::code::event_code_size> events{};
	};

	static bool is_initialsed{ false };
	static event_system_state state{};

	void event::create()
	{
		if (!is_initialsed)
		{
			state = event_system_state{};
			return;
		}

		LOG_WARN("Already created the event system");
	}

	bool event::register_event(code code, void* listener, const callback& callback)
	{
		state.events[(size_t)code].event.emplace_back(listener, callback);

		return true;
	}
	bool event::unregister_event(code code, void* listener, const callback& callback)
	{
		auto& events = state.events.at((size_t)code).event;

		auto event = registered_event{ listener, callback };
		
		auto removed_event = std::remove(events.begin(), events.end(), event);

		if (removed_event != events.end())
		{
			LOG_INFO("Event unregistered");
			return true;
		}

		LOG_ERROR("Attempted to remove event that wasn't registered");
		return false;
	}
	void event::fire_event(code code, void* sender, const context& context)
	{
		const auto& events = state.events.at((size_t)code).event;

		for (const auto& event : events)
		{
			auto result = event.callback(code, sender, event.listener, context);

			if (result)
			{
				break;
			}
		}
	}
}
