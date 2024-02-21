#pragma once

#include <pch.h>

namespace egkr
{
	struct frame_data;

	namespace audio
	{
		class plugin
		{
		public:
			struct configuration
			{
				uint32_t max_sources{};
				uint32_t max_buffers{};
				uint32_t frequency{};
				uint32_t channel_count{};
				uint32_t chunk_size{};
				uint32_t audio_channel_count{};
			};

			virtual ~plugin() = default;
			virtual bool init(const configuration& configuration) = 0;
			virtual bool shutdown() = 0;

			virtual bool update() = 0;

			virtual float3 get_listener_position() const = 0;
			virtual bool set_listener_position(const float3& position) = 0;

			virtual std::pair<const float3&, const float3&> get_orientation() const = 0;
			virtual bool set_orientation(const float3& forward, const float3& up) = 0;

			virtual float get_source_gain(uint32_t source_id) const = 0;
			virtual bool set_source_gain(uint32_t source_id, float gain) = 0;

			virtual float get_source_pitch(uint32_t source_id) const = 0;
			virtual bool set_source_pitch(uint32_t source_id, float pitch) = 0;

			virtual bool get_looping(uint32_t source_id) const = 0;
			virtual bool set_looping(uint32_t source_id, bool loop) = 0;

			virtual const float3& get_source_position(uint32_t source_id) const = 0;
			virtual bool set_source_position(uint32_t source_id, const float3& position) = 0;

			virtual audio::file* load_chunk(const std::string& name) = 0;
			virtual audio::file* load_stream(const std::string& name) = 0;

			virtual void unload_audio(audio::file* file) = 0;
			virtual bool play_source(int8_t source_index) = 0;
			virtual bool play_on_source(audio::file* file, int8_t source_index) = 0;

			virtual void stop_source(int8_t source_index) = 0;
			virtual void pause_source(int8_t source_index) = 0;
			virtual void resume_source(int8_t source_index) = 0;
		};
	}
}
