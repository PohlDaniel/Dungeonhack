#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include <vector>
#include <OgreVector3.h>

using namespace std;
using namespace Ogre;

#define MAX_SOUNDS              14      /* +2 = 16, typ. max of sources */
#define SOUND_POS_DIVISOR       5000
#define STREAMING_CHUNK_SIZE    32768

class GameSoundObject;

class SoundManager : public Ogre::Singleton<SoundManager>
{
public:
    SoundManager() : m_Music(NULL), m_OutdoorAmbient(NULL)
    { }
    virtual ~SoundManager() { }

    virtual void Update(float MoveFactor) = 0;

    virtual GameSoundObject* playSound(const string& filename, float gain, bool loop) = 0;
    virtual GameSoundObject* playPositionedSound(const string& filename, Vector3 Pos, float gain, float radius, bool loop) = 0;

    virtual GameSoundObject* playStreamingMusic(const string& filename) = 0;
    virtual GameSoundObject* playStreamingOutdoorAmbient(const string& filename) = 0;

    static SoundManager& getSingleton(void)
    {
        assert(ms_Singleton);
        return *ms_Singleton;
    }
    static SoundManager* getSingletonPtr(void)
    { return ms_Singleton; }

    void manageSound(GameSoundObject *sound);

    GameSoundObject* getMusic() { return m_Music; }
    GameSoundObject* getOutdoorAmbient() { return m_OutdoorAmbient; }

protected:
    GameSoundObject* m_Music;
    GameSoundObject* m_OutdoorAmbient;

    vector<GameSoundObject*> m_Sounds;
    vector<GameSoundObject*> m_ManagedSounds;
};

#endif // SOUNDMANAGER_H
