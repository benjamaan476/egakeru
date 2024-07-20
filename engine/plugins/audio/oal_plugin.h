#pragma once

#include <resources/audio.h>
#include "audio_plugin.h"

namespace egkr::audio
{
	class oal : public audio::plugin
	{
	public:
		virtual ~oal() override = default;
		bool init(const configuration& configuration) override;
		bool shutdown() override;

		bool update() override;

		float3 get_listener_position() const override;
		bool set_listener_position(const float3& position) override;

		std::pair<const float3&, const float3&> get_orientation() const override;
		bool set_orientation(const float3& forward, const float3& up) override;

		float get_source_gain(uint32_t source_id) const override;
		bool set_source_gain(uint32_t source_id, float gain) override;

		float get_source_pitch(uint32_t source_id) const override;
		bool set_source_pitch(uint32_t source_id, float pitch) override;

		bool get_looping(uint32_t source_id) const override;
		bool set_looping(uint32_t source_id, bool loop) override;

		const float3& get_source_position(uint32_t source_id) const override;
		bool set_source_position(uint32_t source_id, const float3& position) override;

		audio::file* load_chunk(const std::string& name) override;
		audio::file* load_stream(const std::string& name) override;

		void unload_audio(audio::file* file) override;
		bool play_source(uint8_t source_index) override;
		bool play_on_source(audio::file* file, uint8_t source_index) override;

		void stop_source(uint8_t source_index) override;
		void pause_source(uint8_t source_index) override;
		void resume_source(uint8_t source_index) override;
	private:
		static bool check_error();
		uint32_t find_free_buffer();
	};
}
