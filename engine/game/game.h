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
		virtual void update() = 0;
		virtual void render() = 0;
		virtual void resize() = 0;

		const auto& get_application_configuration() const
		{
			return application_configuration_;
		}

	private:
		application_configuration application_configuration_{};
	};
}