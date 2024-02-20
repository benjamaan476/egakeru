#include "audio_system.h"

#include <resources/geometry.h>
#include <resources/audio.h>

#include <plugins/audio/audio_plugin.h>

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

		plugin_->init(plugin_configuration);
	}

	void audio_system::shutdown()
	{
		plugin_->shutdown();
	}

	bool audio_system::update(frame_data* frame_data)
	{
		for (uint32_t i{ 0 }; i < channels_.size(); ++i)
		{
			if (channels_[i].emitter)
			{
				plugin_->set_source_position(i, channels_[i].emitter->position);
				plugin_->set_looping(i, channels_[i].emitter->looping);
				plugin_->set_source_gain(i, channels_[i].emitter->volume * master_volume_ * channels_[i].volume);
			}
		}
		return plugin_->update(frame_data);
	}

	bool audio_system::set_listener_orientation(const float3& position, const float3& forward, const float3& up)
	{
		plugin_->set_listener_position(position);
		plugin_->set_orientation(forward, up);
		return true;
	}

	file* audio_system::load_chunk(const std::string& name)
	{
		return plugin_->load_chunk(name);
	}

	file* audio_system::load_stream(const std::string& name)
	{
		return plugin_->load_stream(name);
	}

	void audio_system::close(file* file)
	{
		plugin_->unload_audio(file);
	}

	bool audio_system::set_master_volume(float volume)
	{
		master_volume_ = glm::clamp(volume, 0.F, 1.F);
		for (uint32_t i{ 0 }; i < channels_.size(); ++i)
		{
			float mix = master_volume_ * channels_[i].volume;
			if (channels_[i].emitter)
			{
				mix *= channels_[i].emitter->volume;
			}

			plugin_->set_source_gain(i, mix);
		}

		return true;
	}

	float audio_system::get_master_volume() const
	{
		return master_volume_;
	}

	bool audio_system::set_channel_volume(int8_t channel_id, float volume)
	{
		channels_[channel_id].volume = volume;
		float mix = master_volume_ * channels_[channel_id].volume;
		if (channels_[channel_id].emitter)
		{
			mix *= channels_[channel_id].emitter->volume;
		}

		plugin_->set_source_gain(channel_id, mix);

		return true;
	}

	float audio_system::get_channel_volume(int8_t channel_id) const
	{
		return channels_[channel_id].volume;
	}

	bool audio_system::play_channel(int8_t channel_id, file* file, bool loop)
	{
		if (channel_id == -1)
		{
			for (uint32_t i{}; i < channels_.size(); ++i)
			{
				if (!channels_[i].current && !channels_[i].emitter)
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

		channels_[channel_id].emitter = nullptr;
		channels_[channel_id].current = file;

		plugin_->set_source_gain(channel_id, channels_[channel_id].volume);

		if (file->type == type::sound_effect)
		{
			const float3 position = plugin_->get_listener_position();
			plugin_->set_source_position(channel_id, position);
			plugin_->set_looping(channel_id, loop);
		}

		plugin_->stop_source(channel_id);

		return plugin_->play_on_source(file, channel_id);
	}

	bool audio_system::play_emitter(int8_t channel_id, emitter* emitter)
	{
		if (channel_id == -1)
		{
			for (uint32_t i{}; i < channels_.size(); ++i)
			{
				if (!channels_[i].current && !channels_[i].emitter)
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

		channels_[channel_id].emitter = emitter;
		channels_[channel_id].current = emitter->audio_file;

		return plugin_->play_on_source(emitter->audio_file, channel_id);
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