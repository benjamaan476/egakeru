#pragma once
#include "pch.h"

#include <systems/system.h>
#include "renderer/camera.h"

namespace egkr
{
	struct camera_system_configuration
	{
		uint16_t max_registered_cameras{ 16 };
	};

	constexpr static std::string_view DEFAULT_CAMERA_NAME{ "default" };

	class renderer_frontend;
	class camera_system
	{
	public:
		using unique_ptr = std::unique_ptr<camera_system>;
		static bool create(const renderer_frontend* renderer_context, const camera_system_configuration& configuration);

		camera_system(const renderer_frontend* renderer_context, const camera_system_configuration& configuration);

		static bool init();
		static bool shutdown();


		static camera::shared_ptr acquire(std::string_view name);
		static void release(const camera::shared_ptr camera);
		static camera::shared_ptr get_default();

	private:
		static void create_default();
	private:
		const renderer_frontend* renderer_context_{};
		camera_system_configuration configuration_{};

		std::unordered_map<std::string, uint32_t> camera_id_by_name_{};
		egkr::vector<camera::shared_ptr> cameras_{};

		camera::shared_ptr default_camera_{};
	};

}
