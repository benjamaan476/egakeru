#pragma once
#include <pch.h>

#include <resources/light.h>
#include <systems/system.h>

namespace egkr
{
		class light_system : public system
		{
		public:
			using light_reference = uint32_t;
			using unique_ptr = std::unique_ptr<light_system>;
			
			static light_system* create();

			light_system();
			~light_system() override;

			bool init() override;
			bool update(float delta_time) override;
			bool shutdown() override;

			static bool add_directional_light(const std::shared_ptr<light::directional_light>& light);
			static bool remove_directional_light();
			static light_reference add_point_light(const light::point_light& light);
			static bool remove_point_light(light_reference light);

			static int32_t point_light_count();
			static light::directional_light* get_directional_light();
			static const std::vector<light::point_light>& get_point_lights();

		private:
			int32_t max_point_light_count_{};
			std::vector<light::point_light> point_lights_{};
			std::shared_ptr<light::directional_light> directional_light_{};
		};
}