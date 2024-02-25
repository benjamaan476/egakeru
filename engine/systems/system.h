#pragma once

#include <pch.h>

namespace egkr
{
	enum class system_type
	{
		console,
		event,
		input,
		platform,
		resource,
		shader,
		renderer,
		job,
		texture,
		font,
		camera,
		render_view,
		material,
		geometry,
		light,

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
		virtual bool update(float delta_time) = 0;

		virtual ~system() = default;
	};

	class game;
	class system_manager
	{
	public:
		static void create(game* game);
		explicit system_manager(game* game);

		static bool init();
		static bool update(float delta_time);
		static void update_input();

		static void shutdown();

	private:
		void register_known(game* game);
		void register_extension();
		void register_user(); 

		static void shutdown_known();
		static void shutdown_extension();
		static void shutdown_user();
		std::unordered_map<system_type, system*> registered_systems_{};
	};
}
