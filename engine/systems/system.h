#pragma once

#include <pch.h>
namespace egkr
{
	enum class system_type
	{
		console,
		evar,
		event,
		input,
		platform,
		resource,
		shader,
		renderer_system,
		job,
		texture,
		font,
		camera,
		render_view,
		material,
		geometry,
		light,
		audio,

		known_max = 255,

		user_max = 512,
		max = user_max
	};

	class system
	{
	public:
		using unique_ptr = std::unique_ptr<system>;
		virtual bool init() = 0;
		virtual bool shutdown() = 0;
		virtual bool update(const frame_data& /*frame_data*/) { return true; }

		virtual ~system() = default;
	};

	class application;
	class system_manager
	{
	public:
		static void create(application* application);
		explicit system_manager(application* application);

		static bool init();
		static bool update(const frame_data& frame_data);
		static void update_input(const frame_data& frame_data);

		static void shutdown();

	private:
		void register_known(application* application);
		void register_extension();
		void register_user(); 

		static void shutdown_known();
		static void shutdown_extension();
		static void shutdown_user();
		std::unordered_map<system_type, system*> registered_systems_;
	};
}
