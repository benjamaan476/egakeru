#include "audio_system.h"

#include <resources/geometry.h>
#include <resources/audio.h>

#include <plugins/audio/audio_plugin.h>
#include <plugins/audio/oal_plugin.h>

namespace egkr::audio
{
	static audio_system::unique_ptr state{};

	bool audio_system::create(const system_configuration& configuration)
	{
		state = std::make_unique<audio_system>(configuration);
		return true;
	}

	audio_system::audio_system(const system_configuration& configuration)
		:configuration_{ configuration }
	{
		if (configuration.audio_channel_count < 4)
		{
			LOG_WARN("Not enough audio channels specified. Setting to 4");
			configuration_.audio_channel_count = 4;
		}

		if (configuration_.chunk_size == 0)
		{
			configuration_.chunk_size = 4096 * MAX_AUDIO_CHANNELS;
		}

		plugin::configuration plugin_configuration
		{
			.max_sources = configuration_.audio_channel_count,
			.max_buffers = 256,
			.frequency = configuration_.frequency,
			.channel_count = configuration_.channel_count,
			.chunk_size = configuration_.chunk_size
		};

		plugin_ = new audio::oal();
		plugin_->init(plugin_configuration);
	}

	void audio_system::shutdown()
	{
		state->plugin_->shutdown();
	}

	bool audio_system::update(geometry::frame_data* /*frame_data*/)
	{
		for (uint32_t i{ 0 }; i < state->channels_.size(); ++i)
		{
			if (state->channels_[i].emitter)
			{
				state->plugin_->set_source_position(i, state->channels_[i].emitter->position);
				state->plugin_->set_looping(i, state->channels_[i].emitter->looping);
				state->plugin_->set_source_gain(i, state->channels_[i].emitter->volume * state->master_volume_ * state->channels_[i].volume);
			}
		}
		return state->plugin_->update();
	}

	bool audio_system::set_listener_orientation(const float3& position, const float3& forward, const float3& up)
	{
		state->plugin_->set_listener_position(position);
		state->plugin_->set_orientation(forward, up);
		return true;
	}

	file* audio_system::load_chunk(const std::string& name)
	{
		return state->plugin_->load_chunk(name);
	}

	file* audio_system::load_stream(const std::string& name)
	{
		return state->plugin_->load_stream(name);
	}

	void audio_system::close(file* file)
	{
		plugin_->unload_audio(file);
	}

	bool audio_system::set_master_volume(float volume)
	{
		state->master_volume_ = glm::clamp(volume, 0.F, 1.F);
		for (uint32_t i{ 0 }; i < state->configuration_.audio_channel_count; ++i)
		{
			float mix = state->master_volume_ * state->channels_[i].volume;
			if (state->channels_[i].emitter)
			{
				mix *= state->channels_[i].emitter->volume;
			}

			state->plugin_->set_source_gain(i, mix);
		}

		return true;
	}

	float audio_system::get_master_volume()
	{
		return state->master_volume_;
	}

	bool audio_system::set_channel_volume(int8_t channel_id, float volume)
	{
		state->channels_[channel_id].volume = volume;
		float mix = state->master_volume_ * state->channels_[channel_id].volume;
		if (state->channels_[channel_id].emitter)
		{
			mix *= state->channels_[channel_id].emitter->volume;
		}

		state->plugin_->set_source_gain(channel_id, mix);

		return true;
	}

	float audio_system::get_channel_volume(int8_t channel_id)
	{
		return state->channels_[channel_id].volume;
	}

	bool audio_system::play_channel(int8_t channel_id, file* file, bool loop)
	{
		if (channel_id == -1)
		{
			for (uint32_t i{}; i < state->channels_.size(); ++i)
			{
				if (!state->channels_[i].current && !state->channels_[i].emitter)
				{
					channel_id = i;
					break;
				}
			}
		}

		if (channel_id == -1)
		{
			LOG_WARN("Could not find channel to play file on");
			return false;
		}

		state->channels_[channel_id].emitter = nullptr;
		state->channels_[channel_id].current = file;

		state->plugin_->set_source_gain(channel_id, state->channels_[channel_id].volume);

		if (file->type == type::sound_effect)
		{
			const float3 position = state->plugin_->get_listener_position();
			state->plugin_->set_source_position(channel_id, position);
			state->plugin_->set_looping(channel_id, loop);
		}

		state->plugin_->stop_source(channel_id);

		return state->plugin_->play_on_source(file, channel_id);
	}

	bool audio_system::play_emitter(int8_t channel_id, emitter* emitter)
	{
		if (channel_id == -1)
		{
			for (uint32_t i{}; i < state->channels_.size(); ++i)
			{
				if (!state->channels_[i].current && !state->channels_[i].emitter)
				{
					channel_id = i;
					break;
				}
			}
		}

		if (channel_id == -1)
		{
			LOG_WARN("Could not find channel to play file on");
			return false;
		}

		state->channels_[channel_id].emitter = emitter;
		state->channels_[channel_id].current = emitter->audio_file;

		return state->plugin_->play_on_source(emitter->audio_file, channel_id);
	}

	void audio_system::stop(int8_t channel_id)
	{
		if (channel_id < 0)
		{
			for (uint32_t i{}; i < channels_.size(); ++i)
			{
				plugin_->stop_source(i);
			}
		}
		else
		{
			plugin_->stop_source(channel_id);
		}
	}

	void audio_system::pause(int8_t channel_id)
	{
		if (channel_id < 0)
		{
			for (uint32_t i{}; i < channels_.size(); ++i)
			{
				plugin_->pause_source(i);
			}
		}
		else
		{
			plugin_->pause_source(channel_id);
		}

	}

	void audio_system::resume(int8_t channel_id)
	{
		if (channel_id < 0)
		{
			for (uint32_t i{}; i < channels_.size(); ++i)
			{
				plugin_->resume_source(i);
			}
		}
		else
		{
			plugin_->resume_source(channel_id);
		}

	}
}