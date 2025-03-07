#pragma once

#include "pch.h"
#include "systems/system.h"
#include "ui/std_ui.h"

namespace egkr
{
	class ui_system : public system
	{
	public:
		struct configuration
		{

		};

		using unique_ptr = std::unique_ptr<ui_system>;
		static ui_system* create(const configuration& configuration);
		explicit ui_system(const configuration& configuration);

		~ui_system() override = default;

		bool init() override;
		bool shutdown() override;
		bool update(const frame_data& frame_data) override;

		static bool render(const frame_data& frame_data);
		static bool register_control(const sui::control::shared_ptr& control);

	private:
		std::vector<sui::control::shared_ptr> registered_controls;
		std::map<std::string, size_t> registered_controls_by_name;
	};
}
