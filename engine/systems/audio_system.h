#pragma once

#include <pch.h>

#include <resources/audio.h>

constexpr static const uint32_t MAX_AUDIO_CHANNELS = 16u;

namespace egkr
{
	namespace geometry
	{
		struct frame_data;
	}

	namespace audio
	{
		class plugin;

		struct system_configuration
		{
			plugin* plugin{};
			uint32_t frequency{};
			uint32_t channel_count{};
			uint32_t chunk_size{};
			uint32_t audio_channel_count{};
		};

		struct channel
		{
			float volume{ 1.F };
			file* current{};
			emitter* emitter{};
		};

		class audio_system
		{
		public:
			using unique_ptr = std::unique_ptr<audio_system>;
			static bool create(const system_configuration& configuration);

			explicit audio_system(const system_configuration& configuration);

			static void shutdown();

			static bool update(geometry::frame_data* frame_data);

			static bool set_listener_orientation(const float3& position, const float3& forward, const float3& up);

			static file* load_chunk(const std::string& name);
			static file* load_stream(const std::string& name);

			void close(file* file);

			static bool set_master_volume(float volume);
			static float get_master_volume();

			static bool set_channel_volume(int8_t channel_id, float volume);
			static float get_channel_volume(int8_t channel_id);

			static bool play_channel(int8_t channel_id, file* file, bool loop);
			static bool play_emitter(int8_t channel_id, emitter* emitter);

			void stop(int8_t channel_id);
			void pause(int8_t channel_id);
			void resume(int8_t channel_id);

		private:
			system_configuration configuration_{};
			plugin* plugin_{};
			float master_volume_{1.F};
			std::array<channel, MAX_AUDIO_CHANNELS> channels_{};
		};
	}
}