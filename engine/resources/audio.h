#pragma once

#include <pch.h>
#include <resources/resource.h>

namespace egkr
{
	namespace audio
	{
		struct plugin_data;

		enum class type
		{
			sound_effect,
			music_stream
		};

		class file
		{
		public:
			type type;
			resource::shared_ptr audio_resource;
			uint32_t format{};
			int32_t channels{};
			uint32_t sample_rate{};
			uint32_t total_samples_left{};
			plugin_data* plugin_data{};
			
			uint64_t load_samples(uint32_t chunk_size, int32_t count);
			void* stream_buffer_data();
			void rewind();
		};

		struct emitter
		{
			float3 position{};
			float volume{1.F};
			float falloff{};
			bool looping{};
			file* audio_file{};
			uint32_t source_id{};
		};
	}
}