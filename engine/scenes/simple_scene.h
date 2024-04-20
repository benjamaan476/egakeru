#pragma once
#include "pch.h"

namespace egkr
{
	namespace scene
	{
		enum class state
		{
			uninitialised,
			initialised,
			loading,
			loaded,
			unloaded
		};

		struct configuration
		{

		};

		class simple_scene
		{
		public:
			using unique_ptr = std::unique_ptr<simple_scene>;
			static unique_ptr create(const configuration& configuration);

			explicit simple_scene(const configuration& configuration);

			void init();
			void destroy();
			void load();
			void unload();

			void update(float delta_time);
		private:
			uint32_t id_{};
			state state_{state::uninitialised};
			bool enabled_{};
		};
	}
}