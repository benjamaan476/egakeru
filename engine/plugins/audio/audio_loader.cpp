#include "audio_loader.h"
#include <resources/audio.h>

#include <stb_vorbis.c>

//#define MINIMP3_IMPLEMENTATION
//#include <minimp3/minimp3_ex.h>

//static mp3dec_t decoder;

namespace egkr
{
	struct audio_file_internal
	{
		stb_vorbis* vorbis{};
		//mp3dec_file_info_t mp3_info{};
		int16_t* pcm{};
		uint64_t pcm_size{};
	};

	uint64_t audio::file::load_samples(uint32_t chunk_size, int32_t /*count*/)
	{
		if (internal_data->vorbis)
		{
			int64_t samples = stb_vorbis_get_samples_short_interleaved(internal_data->vorbis, channels, internal_data->pcm, chunk_size);
			return samples * channels;
		}
		//else if (internal_data->mp3_info.buffer)
		//{
			//return min(total_samples_left, chunk_size);
		//}

		return invalid_64_id;
	}

	void* audio::file::stream_buffer_data()
	{
		if (internal_data->vorbis)
		{
			return internal_data->pcm;
		}
		//else if (internal_data->mp3_info.buffer)
		//{
			//uint64_t pos = internal_data->mp3_info.samples - total_samples_left;
			//return internal_data->mp3_info.buffer + pos;
		//}
		else
		{
			LOG_ERROR("Error streaming audio dta: unknown type");
			return nullptr;
		}
	}

	void audio::file::rewind()
	{
		if (internal_data->vorbis)
		{
			stb_vorbis_seek_start(internal_data->vorbis);
			total_samples_left = stb_vorbis_stream_length_in_samples(internal_data->vorbis) * channels;
		}
		//else if (internal_data->mp3_info.buffer)
		//{
			//total_samples_left = internal_data->mp3_info.samples;
		//}
		else
		{
			LOG_ERROR("Error rewinding audio file: unknown type");
		}
	}

	audio_loader::unique_ptr audio_loader::create(const loader_properties& properties)
	{
		return std::make_unique<audio_loader>(properties);
	}

	audio_loader::audio_loader(const loader_properties& properties)
		: resource_loader{resource_type::audio, properties}
	{
		//mp3dec_init(&decoder);
	}

	resource::shared_ptr audio_loader::load(std::string_view name, void* params)
	{
		auto* parameters = std::bit_cast<audio_resource_loader_params*>(params);

		auto base_path = get_base_path();

		constexpr std::string_view format_string{ "%s/%s" };
		char buff[512]{};
		sprintf_s(buff, format_string.data(), base_path.data(), name.data());

		audio::file* resource_data = new audio::file();
		resource_data->internal_data = new audio_file_internal();
		resource_data->type = parameters->type;
		if (name.contains(".ogg"))
		{
			LOG_TRACE("Processing OGG file");
			int32_t ogg_error{};

			resource_data->internal_data->vorbis = stb_vorbis_open_filename(buff, &ogg_error, nullptr);
			if (!resource_data->internal_data->vorbis)
			{
				LOG_ERROR("Failed to load vorbis file: {}", ogg_error);
				return nullptr;
			}
			stb_vorbis_info info = stb_vorbis_get_info(resource_data->internal_data->vorbis);
			resource_data->channels = info.channels;
			resource_data->sample_rate = info.sample_rate;
			resource_data->total_samples_left = stb_vorbis_stream_length_in_samples(resource_data->internal_data->vorbis);

			if (resource_data->type == audio::type::music_stream)
			{
				uint64_t buffer_lenght = parameters->chunk_size * sizeof(int16_t);
				resource_data->internal_data->pcm = (int16_t*)malloc(buffer_lenght);
				resource_data->internal_data->pcm_size = buffer_lenght;
			}
			else
			{
				uint64_t lenght_samples = stb_vorbis_stream_length_in_samples(resource_data->internal_data->vorbis) * info.channels;
				uint64_t buffer_lenght = lenght_samples * sizeof(int16_t);

				resource_data->internal_data->pcm = (int16_t*)malloc(buffer_lenght);
				resource_data->internal_data->pcm_size = buffer_lenght;
				int32_t read_samples = stb_vorbis_get_samples_short_interleaved(resource_data->internal_data->vorbis, info.channels, resource_data->internal_data->pcm, lenght_samples);
				if ((uint64_t)read_samples != lenght_samples)
				{
					LOG_WARN("Read/length mismatch while reading ogg file.");
				}

				lenght_samples += (lenght_samples % 4);
				resource_data->total_samples_left = lenght_samples;
			}
		}
		else if (name.contains(".mp3"))
		{
			LOG_TRACE("Processing mp3 file");
			//mp3dec_load(&decoder, buff, &resource_data->internal_data->mp3_info, 0, nullptr);
			//mp3dec_file_info_t& info = resource_data->internal_data->mp3_info;
			//resource_data->channels = info.channels;
			//resource_data->sample_rate = info.hz;
			//resource_data->total_samples_left = info.samples;
		}
		else
		{
			LOG_ERROR("Unsupported audio file type");
			return nullptr;
		}

		resource_properties audio_properties{};
		audio_properties.type = resource_type::audio;
		audio_properties.name = name;
		audio_properties.full_path = buff;

		audio_properties.data = new(audio::file);
		*(audio::file*)audio_properties.data = *resource_data;

		return std::make_shared<resource>(audio_properties);
	}

	bool audio_loader::unload(const resource::shared_ptr& resource)
	{
		if (resource->data)
		{
			audio::file* file = (audio::file*)resource->data;

			if (file->internal_data)
			{
				if (file->internal_data->vorbis)
				{
					stb_vorbis_close(file->internal_data->vorbis);
				}
				//else if (file->internal_data->mp3_info.buffer)
				//{
//
				//}
				if (file->internal_data->pcm)
				{
					delete file->internal_data->pcm;
					file->internal_data->pcm = nullptr;
				}

				delete file->internal_data;
			}
		}
			return true;
	}

}