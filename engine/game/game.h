#pragma once
#include "pch.h"

#include "application/application_configuration.h"

namespace egkr
{
	class game
	{
	public:
		using unique_ptr = std::unique_ptr<game>;
		API explicit game(application_configuration configuration);

		API virtual ~game()  = default;

		virtual bool init() = 0;
		virtual void update(std::chrono::milliseconds delta_time) = 0;
		virtual void render(std::chrono::milliseconds delta_time) = 0;
		virtual void resize(uint32_t width, uint32_t height) = 0;

		[[nodiscard]] const auto& get_application_configuration() const
		{
			return application_configuration_;
		}

	private:
		application_configuration application_configuration_{};
	};
}