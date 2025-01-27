#pragma once

#include <pch.h>

#include <systems/system.h>
#include <resources/audio.h>

constexpr static const uint32_t MAX_AUDIO_CHANNELS = 16u;


namespace egkr::audio
{
	class plugin;

	struct system_configuration
	{
		uint32_t frequency{};
		uint32_t channel_count{};
		uint32_t chunk_size{};
		uint32_t audio_channel_count{};
	};

	struct channel
	{
		float volume{ 1.F };
		file* current{};
		emitter* audio_emitter{};
	};

	class audio_system : public system
	{
		public:
			using unique_ptr = std::unique_ptr<audio_system>;
			static audio_system* create(const system_configuration& configuration);

			explicit audio_system(const system_configuration& configuration);

			bool init() override;
			bool update(const frame_data& frame_data) override;
			bool shutdown() override;

			static bool set_listener_orientation(const float3& position, const float3& forward, const float3& up);

			static file* load_chunk(const std::string& name);
			static file* load_stream(const std::string& name);

			static void close(file* file);

			static bool set_master_volume(float volume);
			static float get_master_volume();

			static bool set_channel_volume(uint8_t channel_id, float volume);
			static float get_channel_volume(uint8_t channel_id);

			static bool play_channel(uint8_t channel_id, file* file, bool loop);
			static bool play_emitter(uint8_t channel_id, emitter* emitter);

			void stop(uint8_t channel_id);
			void pause(uint8_t channel_id);
			void resume(uint8_t channel_id);

		private:
			system_configuration configuration_{};
			plugin* plugin_{};
			float master_volume_{1.F};
			std::array<channel, MAX_AUDIO_CHANNELS> channels_{};
	};
}

