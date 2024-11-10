#pragma once

#include <pch.h>
#include <resources/resource.h>

namespace egkr
{
	struct audio_file_internal;
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
			type audio_type;
			resource::shared_ptr audio_resource;
			uint32_t format{};
			int32_t channels{};
			uint32_t sample_rate{};
			uint64_t total_samples_left{};
			plugin_data* data{};
			audio_file_internal* internal_data{};
			
			uint64_t load_samples(uint32_t chunk_size, uint32_t count);
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

	struct audio_resource_loader_params
	{
		audio::type type{};
		uint64_t chunk_size{};
	};
}
