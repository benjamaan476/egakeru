#pragma once

#include <resources/audio.h>
#include "audio_plugin.h"

namespace egkr::audio
{
	struct ALCdevice;
	struct ALCcontext;
	typedef unsigned int ALuint;
	typedef unsigned int ALCuint;

	struct source
	{
		ALCuint id{};
		float gain{};
		float pitch{};
		float3 position{};
		bool looping{};
		bool in_use{};
		std::jthread thread{};
		std::mutex data_mutex{};
		audio::file* current;
		bool trigger_play{};
		bool trigger_exit{};
	};


	class oal : public audio::plugin
	{
	public:
		bool init(const configuration& configuration) override;
		bool shutdown() override;

		bool update(frame_data* frame_data) override;

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
		bool play_source(int8_t source_index) override;
		bool play_on_source(audio::file* file, int8_t source_index) override;

		void stop_source(int8_t source_index) override;
		void pause_source(int8_t source_index) override;
		void resume_source(int8_t source_index) override;
	private:
		static bool check_error();
		source* create_source();
		void destroy_source();
		uint32_t find_free_buffer();

		static bool stream_music_data(ALuint buffer, audio::file* file);
		static bool update_stream(audio::file* audio, source* source);

		static uint32_t source_worker_thread(void* params);

	private:
		egkr::audio::plugin::configuration configuration_{};
		ALCdevice* device_{};
		ALCcontext* context_{};
		egkr::vector<ALuint> buffers_{};
		float3 listener_position_{};
		float3 listener_forward_{};
		float3 listener_up_{};
		source* sources_{};
	};
}