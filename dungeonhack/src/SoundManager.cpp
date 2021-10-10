#include "DungeonHack.h"
#include "SoundManager.h"

#include <stdlib.h>
#include <iostream>

#include "GameManager.h"
#include "GameConfig.h"
#include "GameState.h"
#include "GameObject.h"
#include "AlObjects.h"

#define SOUND_DIR "../media/audio/"
#define MUSIC_DIR "../media/audio/music/"


// Singleton
template<> SoundManager* Ogre::Singleton<SoundManager>::ms_Singleton = 0;


void SoundManager::manageSound(GameSoundObject *sound)
{
    if(sound &&
       find(m_ManagedSounds.begin(), m_ManagedSounds.end(), sound) == m_ManagedSounds.end())
        m_ManagedSounds.push_back(sound);
}

/**
    Constructor
*/
OpenALManager::OpenALManager()
{
    if(alureInitDevice(NULL, NULL) == ALC_FALSE)
        throw new Exception(Ogre::Exception::ERR_INTERNAL_ERROR, "OpenAL initialization", "SoundManager.cpp");

    for(int i = 0;i < 256;i++)
    {
        ALuint src;
        alGenSources(1, &src);
        if(alGetError() != AL_NO_ERROR)
            break;
        SourcePool.push_back(src);
        FreeSources.push_back(src);
    }

    ALCcontext *ctx = alcGetCurrentContext();
    ALCdevice *device = alcGetContextsDevice(ctx);

    AlLog("Init", string("Opened device: ") + alcGetString(device, ALC_DEVICE_SPECIFIER));
    AlLog("Init", string("Device extensions:\n\t") + alcGetString(device, ALC_EXTENSIONS));
    AlLog("Init", string("Version: ") + alGetString(AL_VERSION));
    AlLog("Init", string("Vendor: ") + alGetString(AL_VENDOR));
    AlLog("Init", string("Renderer: ") + alGetString(AL_RENDERER));
    AlLog("Init", string("Extensions:\n\t") + alGetString(AL_EXTENSIONS));
}


/**
    Destructor
*/
OpenALManager::~OpenALManager()
{
    while (m_Sounds.size() > 0)
    {
        m_Sounds.back()->Delete();
    }
    m_ManagedSounds.clear();

    if(m_OutdoorAmbient)
        m_OutdoorAmbient->Delete();
    m_OutdoorAmbient = 0;
    if(m_Music)
        m_Music->Delete();
    m_Music = 0;

    for(vector<ALuint>::iterator i = SourcePool.begin(),
        end = SourcePool.end();i != end;i++)
        alSourceStop(*i);
    alDeleteSources(SourcePool.size(), &SourcePool[0]);
    SourcePool.clear();

    for(map<string,ALuint>::iterator i = BufferCache.begin(),
        end = BufferCache.end();i != end;i++)
        alDeleteBuffers(1, &i->second);
    BufferCache.clear();

    checkALError();
    alureShutdownDevice();
}


/**
    Update sounds
*/
void OpenALManager::Update(float MoveFactor)
{
    const Camera *theCamera = GameManager::getSingletonPtr()->getMainCamera();
    Vector3 cameraPos(0.f);
    if(theCamera)
    {
        cameraPos = theCamera->getWorldPosition();

        // Orientation of the listener : look at then look up
        const Quaternion &cameraRot = theCamera->getWorldOrientation();
        Vector3 vectorUp = cameraRot.yAxis();
        Vector3 vectorForward = cameraRot.zAxis();
        vectorUp.normalise();
        vectorForward.normalise();

        ALfloat pos[3] = { cameraPos.x, cameraPos.y, cameraPos.z };
        ALfloat orient[6] = {
            vectorForward.x, vectorForward.y, vectorForward.z,
            vectorUp.x, vectorUp.y, vectorUp.z
        };

        alListenerfv(AL_POSITION, pos);
        alListenerfv(AL_ORIENTATION, orient);
        checkALError();
    }

    for(vector<GameSoundObject*>::iterator i = m_Sounds.begin(),
        end = m_Sounds.end();i != end;i++)
    {
        if(!(*i)->hasNoPosition())
            (*i)->setDistanceSq(((*i)->getPosition() - cameraPos).squaredLength());
        (*i)->Update(MoveFactor);
    }

    for(vector<GameSoundObject*>::iterator i = m_ManagedSounds.begin(),
        end = m_ManagedSounds.end();i != end;)
    {
        if((*i)->requestDelete)
        {
            (*i)->Delete();
            i = m_ManagedSounds.erase(i);
            end = m_ManagedSounds.end();
        }
        else i++;
    }

    //Update sound
    if(m_Music != 0)
    {
        m_Music->Update(MoveFactor);
        if(m_Music->requestDelete)
        {
            m_Music->Delete();
            m_Music = 0;
        }
    }

    if(m_OutdoorAmbient != 0)
    {
        m_OutdoorAmbient->Update(MoveFactor);
        if(m_OutdoorAmbient->requestDelete)
        {
            m_OutdoorAmbient->Delete();
            m_OutdoorAmbient = 0;
        }
    }
}

ALuint OpenALManager::loadALSound(const string &name)
{
    ALuint &buffer = BufferCache[name];
    if(buffer == 0)
    {
        string thefilename = SOUND_DIR + name;
        AlLog("Message", "Loading "+name);
        buffer = alureCreateBufferFromFile(thefilename.c_str());
        if(buffer == 0)
        {
            AlLog("Error", "Failed to load "+thefilename+": "+alureGetErrorString());
            return 0;
        }
    }
    return buffer;
}

void OpenALManager::killSound(float distsq, int priority)
{
    GameSoundObject *farthest = 0;
    for(vector<GameSoundObject*>::iterator i = m_Sounds.begin(),
        end = m_Sounds.end();i != end;i++)
    {
        if((*i)->requestDelete)
            continue;

        if(!farthest || (*i)->getPriority() < farthest->getPriority() ||
           ((*i)->getPriority() == farthest->getPriority() &&
            (*i)->getDistanceSq() >= farthest->getDistanceSq()))
            farthest = *i;
    }

    if(farthest)
    {
        if(farthest->getPriority() < priority ||
           (farthest->getPriority() == priority &&
            farthest->getDistanceSq() >= distsq))
            farthest->Stop();
    }
}

GameSoundObject* OpenALManager::playSound(const string& name, float gain, bool doloop)
{
    ALuint buffer = loadALSound(name);

    if(FreeSources.size() == 0)
    {
        killSound(0.f, 0);
        if(FreeSources.size() == 0)
            return 0;
    }
    ALuint src = FreeSources.back();
    FreeSources.pop_back();

    OpenALSoundObject *thisSoundObject = new OpenALSoundObject(src, buffer);
    m_Sounds.push_back(thisSoundObject);

    thisSoundObject->setGain(gain);
    thisSoundObject->setLooping(doloop);
    thisSoundObject->setNoPosition(true);
    thisSoundObject->Init();

    thisSoundObject->Play();
    return thisSoundObject;
}


GameSoundObject* OpenALManager::playPositionedSound(const string& name, Vector3 Pos, float gain, float radius, bool doloop)
{
    ALuint buffer = loadALSound(name);

    const Camera *theCamera = GameManager::getSingletonPtr()->getMainCamera();
    float distance = (Pos - theCamera->getWorldPosition()).squaredLength();
    if(FreeSources.size() == 0)
    {
        killSound(distance, 0);
        if(FreeSources.size() == 0)
            return 0;
    }
    ALuint src = FreeSources.back();
    FreeSources.pop_back();

    OpenALSoundObject *thisSoundObject = new OpenALSoundObject(src, buffer);
    m_Sounds.push_back(thisSoundObject);

    thisSoundObject->setGain(gain);
    thisSoundObject->setPosition(Pos);
    thisSoundObject->setRadius(radius);
    thisSoundObject->setLooping(doloop);
    thisSoundObject->setNoPosition(false);
    thisSoundObject->setDistanceSq(distance);
    thisSoundObject->Init();

    thisSoundObject->Play();
    return thisSoundObject;
}


GameSoundObject* OpenALManager::playStreamingMusic(const string& name)
{
    if(m_Music)
        m_Music->Delete();
    m_Music = 0;

    if (GameManager::getSingletonPtr()->getConfig()->getInteger("enableMusic") == 0)
        return 0;

    if(name == "")
        return 0;
    AlLog("Message", "Playing "+name);

    string thefilename = MUSIC_DIR + name;
    alureStream *stream = alureCreateStreamFromFile(thefilename.c_str(), STREAMING_CHUNK_SIZE, 0, 0);
    if(!stream)
    {
        AlLog("Error", "Failed to open "+thefilename+": "+alureGetErrorString());
        return 0;
    }

    if(FreeSources.size() == 0)
    {
        killSound(0.f, 0);
        if(FreeSources.size() == 0)
        {
            alureDestroyStream(stream, 0, 0);
            return 0;
        }
    }
    ALuint src = FreeSources.back();
    FreeSources.pop_back();

    OpenALSoundStreamObject *newSoundObj;
    m_Music = newSoundObj = new OpenALSoundStreamObject(src, stream);

    GameConfiguration* config = GameManager::getSingletonPtr()->getConfig();
    newSoundObj->setGain(config->getFloat("musicVolume")); // set to music volume
    newSoundObj->setLooping(false);
    newSoundObj->setNoPosition(true);
    newSoundObj->Init();

    newSoundObj->Play();
    return newSoundObj;
}

GameSoundObject* OpenALManager::playStreamingOutdoorAmbient(const string& name)
{
    if(m_OutdoorAmbient)
        m_OutdoorAmbient->Delete();
    m_OutdoorAmbient = 0;

    if (GameManager::getSingletonPtr()->getConfig()->getInteger("enableAmbient") == 0)
        return 0;

    if(name == "")
        return 0;
    AlLog("Message", "Playing "+name);

    string thefilename = SOUND_DIR + name;
    alureStream *stream = alureCreateStreamFromFile(thefilename.c_str(), STREAMING_CHUNK_SIZE, 0, 0);
    if(!stream)
    {
        AlLog("Error", "Failed to open "+thefilename+": "+alureGetErrorString());
        return 0;
    }

    if(FreeSources.size() == 0)
    {
        killSound(0.f, 0);
        if(FreeSources.size() == 0)
        {
            alureDestroyStream(stream, 0, 0);
            return 0;
        }
    }
    ALuint src = FreeSources.back();
    FreeSources.pop_back();

    OpenALSoundStreamObject *newSoundObj;
    m_OutdoorAmbient = newSoundObj = new OpenALSoundStreamObject(src, stream);

    GameConfiguration* config = GameManager::getSingletonPtr()->getConfig();
    newSoundObj->setGain(config->getFloat("ambientVolume")); // set to ambience volume
    newSoundObj->setLooping(false);
    newSoundObj->setNoPosition(true);
    newSoundObj->Init();

    newSoundObj->Play();
    return newSoundObj;
}
