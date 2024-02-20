#include "oal_plugin.h"

#include <AL/al.h>
#include <AL/alc.h>

constexpr static const uint32_t OAL_PLUGIN_MUSIC_BUFFER_COUNT = 2u;

namespace egkr::audio
{
	struct plugin_data
	{
		ALuint buffer_{};
		std::array<ALuint, OAL_PLUGIN_MUSIC_BUFFER_COUNT> buffers_{};
		bool is_looping_{};
	};


	bool oal::check_error()
	{
		return alGetError() != AL_NO_ERROR;
	}

	bool oal::stream_music_data(ALuint buffer, audio::file* file)
	{
		uint64_t size = file->load_samples(configuration_.chunk_size, configuration_.chunk_size);

		if (size == invalid_64_id)
		{
			LOG_ERROR("Invalid audio file");
			return false;
		}

		if (size == 0)
		{
			return false;
		}

		//check_error();

		void* streamed_data = file->stream_buffer_data();

		if (streamed_data)
		{
			alBufferData(buffer, file->format, streamed_data, size * sizeof(ALshort), file->sample_rate);
			//check_error();
		}
		else
		{
			LOG_ERROR("Could not get stream data from audio file");
			return false;
		}

		file->total_samples_left -= size;

		return true;
	}

	bool oal::update_stream(audio::file* audio, source* source)
	{
		ALint source_state{};
		alGetSourcei(source->id, AL_SOURCE_STATE, &source_state);

		if (source_state != AL_PLAYING)
		{
			LOG_TRACE("Need tp play audio file");
			alSourcePlay(source->id);
		}

		ALint processed_buffer_count{};
		alGetSourcei(source->id, AL_BUFFERS_PROCESSED, &processed_buffer_count);

		while (processed_buffer_count--)
		{
			ALuint buffer_id{};
			alSourceUnqueueBuffers(source->id, 1, &buffer_id);

			if (!stream_music_data(buffer_id, audio))
			{
				bool done{ true };
				if (audio->plugin_data->is_looping_)
				{
					audio->rewind();
					done = !stream_music_data(buffer_id, audio);
				}

				if (done)
				{
					return false;
				}
			}

			alSourceQueueBuffers(source->id, 1, &buffer_id);
		}

		return true;
	}

	struct source_worker_thread_params
	{
		audio::plugin* plugin{};
		source* source{};
	};

	uint32_t oal::source_worker_thread(void* params)
	{
		auto* parameters = (source_worker_thread_params*)params;
		auto* plugin = parameters->plugin;
		auto* source = parameters->source;

		delete parameters;

		bool do_break{};
		while (!do_break)
		{
			{
				std::lock_guard lock{ source->data_mutex };
				if (source->trigger_exit)
				{
					do_break = true;
				}

				if (source->trigger_play)
				{
					alSourcePlay(source->id);
					source->trigger_play = false;
				}
			}

			if (source->current && source->current->type == type::music_stream)
			{
				update_stream(source->current, source);
			}

		}
		return 0;
	}
}
