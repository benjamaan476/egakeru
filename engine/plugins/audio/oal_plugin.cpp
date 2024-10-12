#include "oal_plugin.h"

#include <AL/al.h>
#include <AL/alc.h>

#include <systems/resource_system.h>

constexpr static const uint32_t OAL_PLUGIN_MUSIC_BUFFER_COUNT = 2u;

namespace egkr::audio
{
    struct source;
    static bool stream_music_data(ALuint buffer, audio::file* file);
    static bool update_stream(audio::file* audio, source* source);
    struct plugin_internal_data
    {
        egkr::audio::plugin::configuration configuration{};
        ALCdevice* device{};
        ALCcontext* context{};
        egkr::vector<ALuint> buffers;
        egkr::vector<uint32_t> free_buffers;
        egkr::vector<source> sources;
        float3 listener_position{};
        float3 listener_forward{ 1, 0, 0 };
        float3 listener_up{ 0, 0, 1 };
    };
    struct plugin_data
    {
        ALuint buffer{};
        std::array<ALuint, OAL_PLUGIN_MUSIC_BUFFER_COUNT> buffers{};
        bool is_looping{};
    };

    struct work_thread_params
    {
        source* source{};
    };

    struct source
    {
        ALCuint id{};
        float gain{ 1.F };
        float pitch{ 1.F };
        float3 position{};
        bool looping{};
        bool in_use{};
        bool trigger_play{};
        bool trigger_exit{false};
        //std::mutex mutex;
        audio::file* current{};
        std::jthread thread{};

        source()
        {
            alGenSources((ALuint)1, &id);

            work_thread_params* params = new work_thread_params();
            params->source = this;
            thread = std::jthread(&source::worker_thread, params);
        }

        void destroy()
        {
            alDeleteSources(1, &id);
            id = invalid_32_id;
            trigger_exit = true;
        }

        static uint32_t worker_thread(work_thread_params* params)
        {
            source* s = params->source;

            delete params;
            bool do_break{};
            while (!do_break)
            {
                {
                    //std::lock_guard lock{ mutex };
                    if (s->trigger_exit) 
                    {
                        do_break = true;
                    }

                    if (s->trigger_play)
                    {
                        alSourcePlay(s->id);
                        s->trigger_play = false;
                    }
                }

                if (s->current && s->current->type == type::music_stream)
                { 
                    update_stream(s->current, s);
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
            return 0;
        }

        bool play()
        {
            {
                //std::lock_guard lock{ mutex };
                if (current)
                {
                    trigger_play = true;
                    in_use = true;
                }
            }
            return true;
        }

        bool play_on_source(audio::file* file)
        {
            {
                //std::lock_guard lock{ mutex };
                if (file->type == audio::type::sound_effect)
                {
                    alSourceQueueBuffers(id, 1, &file->plugin_data->buffer);
                    //check_error();
                }
                else
                {
                    for (uint32_t i{}; i < OAL_PLUGIN_MUSIC_BUFFER_COUNT; ++i)
                    {
                        if (!stream_music_data(file->plugin_data->buffers[i], file))
                        {
                            LOG_ERROR("Failed to stream data to buffer {} in music file. File load failed", i);
                            break;
                        }
                    }
                    alSourceQueueBuffers(id, OAL_PLUGIN_MUSIC_BUFFER_COUNT, file->plugin_data->buffers.data());
                    //check_error();
                }

                current = file;
                in_use = true;
                alSourcePlay(id);
            }
            return true;
        }
    };


    static plugin_internal_data* internal_data{};

    source create_source();

    static egkr::vector<uint32_t> find_playing_sources()
    {
        egkr::vector<uint32_t> playing{};
        ALint state{};
        for (uint32_t i{}; i < internal_data->configuration.max_sources; ++i)
        {
            alGetSourcei(internal_data->sources[i - 1].id, AL_SOURCE_STATE, &state);
            if (state == AL_PLAYING) { playing.push_back(internal_data->sources[i - 1].id); }
        }
        return playing;
    }

    static void clear_buffer(uint32_t* buffer, uint32_t amount)
    {
        for (uint32_t a{}; a < amount; ++a)
        {
            for (uint32_t i{}; i < internal_data->buffers.size(); ++i)
            {
                if (buffer[a] == internal_data->buffers[i])
                {
                    internal_data->free_buffers.push_back(i);
                    return;
                }
            }
        }
        LOG_WARN("Could not clear buffer");
    }

    bool oal::check_error()
    {
        ALCenum error = alGetError();

        if (error != AL_NO_ERROR)
        {
            LOG_ERROR("OpenAL error {}: {}", error, error);
            return false;
        }
        return true;
    }

    uint32_t oal::find_free_buffer()
    {
        if (internal_data->free_buffers.empty())
        {
            LOG_INFO("No free buffers, attempting to free an existing one");
            if (!check_error()) { return false; }

            const auto playing_sources = find_playing_sources();

            for (uint32_t i{}; i < playing_sources.size(); ++i) { alSourcePause(internal_data->sources[i - 1].id); }

            ALint to_be_freed{};
            ALuint buffers_freed{};
            for (uint32_t i{}; i < internal_data->buffers.size(); ++i)
            {
                alGetSourcei(internal_data->sources[i - 1].id, AL_BUFFERS_PROCESSED, &to_be_freed);

                if (to_be_freed > 0)
                {
                    alSourceUnqueueBuffers(internal_data->sources[i - 1].id, to_be_freed, &buffers_freed);
                    clear_buffer(&buffers_freed, (uint32_t)to_be_freed);
                    alSourcePlay(internal_data->sources[i - 1].id);
                }
            }

            for (uint32_t i{}; i < playing_sources.size(); ++i) { alSourcePlay(internal_data->sources[i - 1].id); }
        }

        if (internal_data->free_buffers.empty())
        {
            LOG_ERROR("Could not find or clear a buffer");
            return invalid_32_id;
        }

        auto buffer = internal_data->free_buffers.back();
        internal_data->free_buffers.pop_back();

        LOG_TRACE("Found free buffer id {}", buffer);
        LOG_INFO("There are now {} free buffers remaining", internal_data->free_buffers.size());


        return buffer;
    }

    bool stream_music_data(ALuint buffer, audio::file* file)
    {
        uint64_t size = file->load_samples(internal_data->configuration.chunk_size, internal_data->configuration.chunk_size);

        if (size == invalid_64u_id)
        {
            LOG_ERROR("Invalid audio file");
            return false;
        }

        if (size == 0) { return false; }

        // check_error();

        void* streamed_data = file->stream_buffer_data();

        if (streamed_data)
        {
            alBufferData(buffer, (ALenum)file->format, streamed_data, (ALsizei)(size * sizeof(ALshort)), (ALsizei)file->sample_rate);
            // check_error();
        }
        else
        {
            LOG_ERROR("Could not get stream data from audio file");
            return false;
        }

        file->total_samples_left -= size;

        return true;
    }

    bool update_stream(audio::file* audio, source* source)
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
                if (audio->plugin_data->is_looping)
                {
                    audio->rewind();
                    done = !stream_music_data(buffer_id, audio);
                }

                if (done) { return false; }
            }

            alSourceQueueBuffers(source->id, 1, &buffer_id);
        }

        return true;
    }

    bool oal::init(const configuration& configuration)
    {
        internal_data = new plugin_internal_data();

        internal_data->configuration = configuration;
        if (internal_data->configuration.max_sources < 1)
        {
            LOG_WARN("Audio plugin max_sources was configured as 0. Defaulting to 8.");
            internal_data->configuration.max_sources = 8;
        }

        if (internal_data->configuration.max_buffers < 20)
        {
            LOG_WARN("Audio plugin max_buffers was configured to be less than 20. Defaulting to 256");
            internal_data->configuration.max_buffers = 256;
        }

        internal_data->buffers.resize(internal_data->configuration.max_buffers);

        internal_data->device = alcOpenDevice(nullptr);
        // check_error();
        if (!internal_data->device)
        {
            LOG_ERROR("Unable to obtain OpenAL device. Initialization failed");
            return false;
        }

        internal_data->context = alcCreateContext(internal_data->device, nullptr);
        // check_error();
        if (!alcMakeContextCurrent(internal_data->context)) { check_error(); }

        alListener3f(AL_VELOCITY, 0, 0, 0);
        check_error();

        internal_data->sources.resize(internal_data->configuration.max_sources);
//        internal_data->sources = (source*)malloc(internal_data->configuration.max_sources * sizeof(source));
//        for (uint32_t i{}; i < internal_data->configuration.max_sources; ++i)
//        {
//            internal_data->sources[i] = source(); 
//        }

        alGenBuffers((ALsizei)internal_data->buffers.size(), internal_data->buffers.data());
        check_error();

        for (const auto& buffer : internal_data->buffers) { internal_data->free_buffers.push_back(buffer); }

        return true;
    }

    bool oal::shutdown()
    {
        if (internal_data)
        {
            for (uint32_t i{}; i < internal_data->configuration.max_sources; ++i) 
            {
                internal_data->sources[i].destroy();
            }

            if (internal_data->device)
            {
                alcCloseDevice(internal_data->device);
                internal_data->device = nullptr;
            }

            delete internal_data;
            internal_data = nullptr;
        }
        return true;
    }

    bool oal::update() { return true; }

    float3 oal::get_listener_position() const { return internal_data->listener_position; }

    bool oal::set_listener_position(const float3& position)
    {
        internal_data->listener_position = position;
        alListener3f(AL_POSITION, position.x, position.y, position.z);
        return check_error();
    }

    std::pair<const float3&, const float3&> oal::get_orientation() const
    {
        return { internal_data->listener_forward, internal_data->listener_up };
    }

    bool oal::set_orientation(const float3& forward, const float3& up)
    {
        internal_data->listener_forward = forward;
        internal_data->listener_up = up;
        ALfloat orientation[] = { forward.x, forward.y, forward.z, up.x, up.y, up.z };
        alListenerfv(AL_ORIENTATION, orientation);
        return check_error();
    }

    float oal::get_source_gain(uint32_t source_id) const { return internal_data->sources[source_id].gain; }

    bool oal::set_source_gain(uint32_t source_id, float gain)
    {
        internal_data->sources[source_id].gain = gain;
        alSourcef(internal_data->sources[source_id].id, AL_GAIN, gain);
        return check_error();
    }

    float oal::get_source_pitch(uint32_t source_id) const { return internal_data->sources[source_id].pitch; }
    bool oal::set_source_pitch(uint32_t source_id, float pitch)
    {
        internal_data->sources[source_id].pitch = pitch;
        alSourcef(internal_data->sources[source_id].id, AL_PITCH, pitch);
        return check_error();
    }

    bool oal::get_looping(uint32_t source_id) const { return internal_data->sources[source_id].looping; }

    bool oal::set_looping(uint32_t source_id, bool loop)
    {
        internal_data->sources[source_id].looping = loop;
        alSourcei(internal_data->sources[source_id].id, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
        return check_error();
    }

    const float3& oal::get_source_position(uint32_t source_id) const { return internal_data->sources[source_id].position; }

    bool oal::set_source_position(uint32_t source_id, const float3& position)
    {
        internal_data->sources[source_id].position = position;
        return true;
    }

    audio::file* oal::load_chunk(const std::string& name)
    {

        audio_resource_loader_params params
        {
            .type = audio::type::sound_effect,
            .chunk_size = (uint64_t)internal_data->configuration.chunk_size
        };
        auto resource = resource_system::load(name, resource::type::audio, &params);

        audio::file* file = (audio::file*)resource->data;
        file->plugin_data = new plugin_data();

        file->plugin_data->buffer = find_free_buffer();

        if (file->plugin_data->buffer == invalid_32_id)
        {
            delete file->plugin_data;
            resource_system::unload(resource);
            LOG_ERROR("Unable to open audio file due to no buffers being available");
            return nullptr;
        }
        check_error();
        file->format = AL_FORMAT_MONO16;
        if (file->channels == 2) { file->format = AL_FORMAT_STEREO16; }

        if (file->total_samples_left > 0)
        {
            void* pcm = file->stream_buffer_data();
            check_error();
            alBufferData(file->plugin_data->buffer, (ALenum)file->format, (int16_t*)pcm, (ALsizei)file->total_samples_left, (ALsizei)file->sample_rate);
            check_error();
            return file;
        }

        if (file && file->plugin_data) { delete file->plugin_data; }
        resource_system::unload(resource);
        return nullptr;
    }

    audio::file* oal::load_stream(const std::string& name)
    {
        audio_resource_loader_params params
        {
            .type = audio::type::music_stream,
            .chunk_size = (uint64_t)internal_data->configuration.chunk_size
        };

        auto resource = resource_system::load(name, resource::type::audio, &params);

        audio::file* file = (audio::file*)resource->data;
        file->plugin_data = new plugin_data();

        for (uint32_t i{}; i < OAL_PLUGIN_MUSIC_BUFFER_COUNT; ++i)
        {
            file->plugin_data->buffers[i] = find_free_buffer();
            if (file->plugin_data->buffers[i] == invalid_32_id)
            {
                LOG_ERROR("Unable to open music file due to no buffers being available");
                return nullptr;
            }
        }

        check_error();
        file->format = AL_FORMAT_MONO16;
        if (file->channels == 2) { file->format = AL_FORMAT_STEREO16; }

        file->plugin_data->is_looping = true;
        return file;
    }

    void oal::unload_audio(audio::file* file)
    {
        clear_buffer(&file->plugin_data->buffer, 0);

        delete file->plugin_data;
        resource_system::unload(file->audio_resource);
    }

    bool oal::play_source(uint8_t source_index)
    {
        return internal_data->sources[source_index].play();
    }

    bool oal::play_on_source(audio::file* file, uint8_t source_index)
    {
        return internal_data->sources[source_index].play_on_source(file);
    }


    void oal::stop_source(uint8_t source_index)
    {
        auto& source = internal_data->sources[source_index];
        alSourceStop(source.id);

        alSourcei(source.id, AL_BUFFER, 0);
        check_error();
        alSourceRewind(source.id);
        source.in_use = false;
    }

    void oal::pause_source(uint8_t source_index)
    {
        auto& source = internal_data->sources[source_index];
        ALint source_state{};
        alGetSourcei(source.id, AL_SOURCE_STATE, &source_state);
        if (source_state == AL_PLAYING) { alSourcePause(source.id); }
    }

    void oal::resume_source(uint8_t source_index)
    {
        auto& source = internal_data->sources[source_index];
        ALint source_state{};
        alGetSourcei(source.id, AL_SOURCE_STATE, &source_state);
        if (source_state == AL_PAUSED) { alSourcePlay(source.id); }
    }
}// namespace egkr::audio
