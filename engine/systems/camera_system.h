#pragma once
#include "pch.h"

#include <systems/system.h>
#include "renderer/camera.h"

namespace egkr
{

	constexpr static std::string_view DEFAULT_CAMERA_NAME{ "default" };

	class renderer_frontend;
	class camera_system : public system
	{
	public:
		struct configuration
		{
			uint16_t max_registered_cameras{ 16 };
		};

		using unique_ptr = std::unique_ptr<camera_system>;
		static camera_system* create(const configuration& configuration);

		explicit camera_system(const configuration& configuration);

		bool init() override;
		bool update(float delta_time) override;
		bool shutdown() override;


		static camera::shared_ptr acquire(std::string_view name);
		static void release(const camera::shared_ptr camera);
		static camera::shared_ptr get_default();

	private:
		static void create_default();
	private:
		configuration configuration_{};

		std::unordered_map<std::string, uint32_t> camera_id_by_name_{};
		egkr::vector<camera::shared_ptr> cameras_{};

		camera::shared_ptr default_camera_{};
	};

}
