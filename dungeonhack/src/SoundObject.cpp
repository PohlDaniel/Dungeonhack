#include "DungeonHack.h"
#include "SoundManager.h"
#include "GameObject.h"
#include "AlObjects.h"

GameSoundObject::GameSoundObject()
{
    entityGroup = 100; //Sounds have group 100
    m_EntityType = "GameSoundObject";
    requestDelete = false;
    isActive = true;

    m_noPosition = true;
    m_radius = 1.f;
    m_gain = 1.f;
    m_priority = 0;
    m_distance2 = 0.f;
}


OpenALSoundObject::OpenALSoundObject(ALuint src, ALuint buf)
  : Source(src), Buffer(buf)
{
}

OpenALSoundObject::~OpenALSoundObject()
{
    Stop();

    OpenALManager *parent = OpenALManager::getSingletonPtr();
    vector<GameSoundObject*>::iterator i;
    i = find(parent->m_Sounds.begin(), parent->m_Sounds.end(), this);
    if(i != parent->m_Sounds.end()) parent->m_Sounds.erase(i);
}

void OpenALSoundObject::Init()
{
    alSourcef(Source, AL_GAIN, m_gain);
    alSourcei(Source, AL_LOOPING, (m_loop?AL_TRUE:AL_FALSE));
    if(m_noPosition)
    {
        alSourcei(Source, AL_SOURCE_RELATIVE, AL_TRUE);
        alSource3f(Source, AL_POSITION, 0.f, 0.f, 0.f);
        alSource3f(Source, AL_VELOCITY, 0.f, 0.f, 0.f);
        alSourcef(Source, AL_REFERENCE_DISTANCE, SOUND_POS_DIVISOR);
        //alSourcef(Source, AL_MAX_DISTANCE, 1.f);
    }
    else
    {
        alSourcei(Source, AL_SOURCE_RELATIVE, AL_FALSE);
        alSource3f(Source, AL_POSITION, m_pos.x, m_pos.y, m_pos.z);
        alSource3f(Source, AL_VELOCITY, m_velocity.x, m_velocity.y,
                                        m_velocity.z);
        alSourcef(Source, AL_REFERENCE_DISTANCE, SOUND_POS_DIVISOR);
        //alSourcef(Source, AL_MAX_DISTANCE, m_radius);
    }
    if(checkALError() != AL_NO_ERROR)
        Stop();
}

void OpenALSoundObject::Play()
{
    if(requestDelete)
        return;

    alSourcei(Source, AL_BUFFER, Buffer);
    alSourcePlay(Source);
    if(checkALError() != AL_NO_ERROR)
        Stop();
}

void OpenALSoundObject::Stop()
{
    if(alIsSource(Source))
    {
        alSourceStop(Source);
        alSourcei(Source, AL_BUFFER, 0);
        checkALError();

        OpenALManager *parent = OpenALManager::getSingletonPtr();
        parent->FreeSources.push_back(Source);
        Source = 0;
    }
    requestDelete = true;
}

void OpenALSoundObject::Update(float MoveFactor)
{
    if(requestDelete)
        return;

    ALint state = AL_PLAYING;
    ALint local = AL_TRUE;
    alGetSourcei(Source, AL_SOURCE_STATE, &state);
    alGetSourcei(Source, AL_SOURCE_RELATIVE, &local);
    checkALError();

    if(state != AL_PLAYING && state != AL_PAUSED)
    {
        Stop();
        return;
    }

    if(local == AL_FALSE)
    {
        alSource3f(Source, AL_POSITION, m_pos.x, m_pos.y, m_pos.z);
        alSource3f(Source, AL_VELOCITY, m_velocity.x, m_velocity.y, m_velocity.z);
#ifdef MUTE_AT_RADIUS
        alSourcef(Source, AL_GAIN, ((m_distance2 > m_radius*m_radius) ? 0.f : m_gain));
#endif
        checkALError();
    }
}

void OpenALSoundObject::Delete()
{
    delete this;
}

void OpenALSoundObject::UpdateVolume()
{
    alSourcef(Source, AL_GAIN, m_gain);
}

OpenALSoundStreamObject::OpenALSoundStreamObject(ALuint src, alureStream *stream)
  : Source(src), Stream(stream)
{
    memset(Buffers, 0, sizeof(Buffers));
    alGenBuffers(NumBuffers, Buffers);
    if(checkALError() != AL_NO_ERROR)
        Stop();
}

OpenALSoundStreamObject::~OpenALSoundStreamObject()
{
    Stop();

    OpenALManager *parent = OpenALManager::getSingletonPtr();
    vector<GameSoundObject*>::iterator i;
    i = find(parent->m_Sounds.begin(), parent->m_Sounds.end(), this);
    if(i != parent->m_Sounds.end()) parent->m_Sounds.erase(i);
}

void OpenALSoundStreamObject::Init()
{
    if(requestDelete)
        return;

    alSourcef(Source, AL_GAIN, m_gain);
    alSourcei(Source, AL_LOOPING, AL_FALSE);
    if(m_noPosition)
    {
        alSourcei(Source, AL_SOURCE_RELATIVE, AL_TRUE);
        alSource3f(Source, AL_POSITION, 0.f, 0.f, 0.f);
        alSource3f(Source, AL_VELOCITY, 0.f, 0.f, 0.f);
        alSourcef(Source, AL_REFERENCE_DISTANCE, SOUND_POS_DIVISOR);
        //alSourcef(Source, AL_MAX_DISTANCE, 1.f);
    }
    else
    {
        alSourcei(Source, AL_SOURCE_RELATIVE, AL_FALSE);
        alSource3f(Source, AL_POSITION, m_pos.x, m_pos.y, m_pos.z);
        alSource3f(Source, AL_VELOCITY, m_velocity.x, m_velocity.y,
                                        m_velocity.z);
        alSourcef(Source, AL_REFERENCE_DISTANCE, SOUND_POS_DIVISOR);
        //alSourcef(Source, AL_MAX_DISTANCE, m_radius);
    }
    if(checkALError() != AL_NO_ERROR)
        Stop();
}

void OpenALSoundStreamObject::Play()
{
    if(requestDelete)
        return;

    ALint ret = alureBufferDataFromStream(Stream, NumBuffers, Buffers);
    if(ret <= 0)
    {
        OpenALManager::AlLog("Error", string("Failed to buffer data from stream: ")+alureGetErrorString());
        Stop();
        return;
    }

    alSourceQueueBuffers(Source, ret, Buffers);
    alSourcePlay(Source);
    if(checkALError() != AL_NO_ERROR)
        Stop();
}

void OpenALSoundStreamObject::Stop()
{
    if(alIsSource(Source))
    {
        alSourceStop(Source);
        alSourcei(Source, AL_BUFFER, 0);
        checkALError();

        OpenALManager *parent = OpenALManager::getSingletonPtr();
        parent->FreeSources.push_back(Source);
        Source = 0;
    }

    if(alureDestroyStream(Stream, NumBuffers, Buffers) == AL_FALSE)
        OpenALManager::AlLog("Error", string("Could not destroy stream: ") + alureGetErrorString());
    memset(Buffers, 0, sizeof(Buffers));
    Stream = NULL;
    requestDelete = true;
}

void OpenALSoundStreamObject::Update(float MoveFactor)
{
    if(requestDelete)
        return;

    ALint state = AL_PLAYING;
    ALint local = AL_TRUE;
    ALint processed = 0;

    alGetSourcei(Source, AL_SOURCE_STATE, &state);
    alGetSourcei(Source, AL_SOURCE_RELATIVE, &local);
    alGetSourcei(Source, AL_BUFFERS_PROCESSED, &processed);
    if(processed > 0)
    {
        ALuint bufs[NumBuffers];
        alSourceUnqueueBuffers(Source, processed, bufs);

        ALint filled = alureBufferDataFromStream(Stream, processed, bufs);
        while(filled < processed)
        {
            if(filled < 0)
            {
                OpenALManager::AlLog("Error", string("Failed to buffer from stream: ") + alureGetErrorString());
                filled = 0;
            }

            if(!m_loop)
            {
                if(state != AL_PLAYING && state != AL_PAUSED)
                {
                    Stop();
                    return;
                }
                break;
            }

            ALint got = -1;
            if(alureRewindStream(Stream) != AL_FALSE)
                got = alureBufferDataFromStream(Stream, processed-filled, &bufs[filled]);
            if(got <= 0)
            {
                OpenALManager::AlLog("Error", string("Failed to loop stream: ") + alureGetErrorString());
                Stop();
                return;
            }
            filled += got;
        }
        alSourceQueueBuffers(Source, filled, bufs);
    }
    if(local == AL_FALSE)
    {
        alSource3f(Source, AL_POSITION, m_pos.x, m_pos.y, m_pos.z);
        alSource3f(Source, AL_VELOCITY, m_velocity.x, m_velocity.y, m_velocity.z);
#ifdef MUTE_AT_RADIUS
        alSourcef(Source, AL_GAIN, ((m_distance2 > m_radius*m_radius) ? 0.f : m_gain));
#endif
    }
    if(state != AL_PLAYING && state != AL_PAUSED)
        alSourcePlay(Source);

    if(checkALError() != AL_NO_ERROR)
        Stop();
}

void OpenALSoundStreamObject::Delete()
{
    delete this;
}

void OpenALSoundStreamObject::UpdateVolume()
{
    alSourcef(Source, AL_GAIN, m_gain);
}
