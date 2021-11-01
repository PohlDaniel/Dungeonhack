#ifndef _AL_OBJECTS_H
#define _AL_OBJECTS_H

#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <OgreVector3.h>

#include <al.h>
#include <alc.h>
#include <alure.h>

#include "SoundManager.h"

static inline ALenum checkALError(const char*,int);

// Define this to mute sounds (set their gains to 0) when they're outside their
// set radius
#define MUTE_AT_RADIUS


class OpenALManager : public SoundManager
{
public:
    OpenALManager();
    virtual ~OpenALManager();

    virtual void Update(float MoveFactor);

    virtual GameSoundObject* playSound(const string& filename, float gain, bool loop);
    virtual GameSoundObject* playPositionedSound(const string& filename, Vector3 Pos, float gain, float radius, bool loop);

    virtual GameSoundObject* playStreamingMusic(const string& filename);
    virtual GameSoundObject* playStreamingOutdoorAmbient(const string& filename);

    static OpenALManager& getSingleton(void)
    {
        assert(ms_Singleton);
        return *static_cast<OpenALManager*>(ms_Singleton);
    }
    static OpenALManager* getSingletonPtr(void)
    { return static_cast<OpenALManager*>(ms_Singleton); }


    static void AlLog(const char *level, const string &str)
    {
        cout << "OpenAL " << level << ": " << str << endl;
    }
protected:
    ALuint loadALSound(const string &name);
    void killSound(float distance, int priority);

    vector<ALuint> SourcePool;

    vector<ALuint> FreeSources;
    map<string,ALuint> BufferCache;

    friend ALenum checkALError(const char*,int);
    friend class OpenALSoundObject;
    friend class OpenALSoundStreamObject;
};


class OpenALSoundObject : public GameSoundObject {
public:
    OpenALSoundObject(ALuint src, ALuint buf);
    virtual ~OpenALSoundObject();

    virtual void Init();
    virtual void Update(float);
    virtual void Delete();

    virtual void Stop();
    void Play();

    void UpdateVolume();

protected:
    ALuint Source;
    ALuint Buffer;
};

class OpenALSoundStreamObject : public GameSoundObject {
public:
    OpenALSoundStreamObject(ALuint src, alureStream *stream);
    virtual ~OpenALSoundStreamObject();

    virtual void Init();
    virtual void Update(float);
    virtual void Delete();

    virtual void Stop();
    void Play();

    void UpdateVolume();

protected:
    static const int NumBuffers = 3;

    ALuint Source;
    ALuint Buffers[NumBuffers];

    alureStream *Stream;
};

#define AL_LOG_ERRORS
#ifdef AL_LOG_ERRORS
static inline ALenum checkALError(const char *s, int l)
{
    ALenum err = alGetError();
    if(err != AL_NO_ERROR)
    {
        stringstream sstream(alGetString(err));
        sstream << " - " << s << ", " << l;
        OpenALManager::AlLog("Error", sstream.str());
    }
    return err;
}
#else
static inline ALenum checkALError(const char*,int)
{ return alGetError(); }
#endif
#define checkALError() checkALError(__FILE__, __LINE__)

#endif
