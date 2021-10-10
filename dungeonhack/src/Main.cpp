#include "DungeonHack.h"
#include <string.h>

#include "GameManager.h"
#include "PhysicsManager.h"
#include "GameWorld.h"
#include "SoundManager.h"
#include "AlObjects.h"

#include "PlayState.h" 
#include "MainmenuState.h"

int checkWorkingDirectory();


int main(int argc, char **argv)
{
    if (!checkWorkingDirectory())
        return 1;

    GameManager* game = new GameManager();
    GameWorld* gameWorld = new GameWorld();
    PhysicsManager* physics = new PhysicsManager();
    SoundManager * soundManager = new OpenALManager();

    try
    {
        if( argc < 2 )
            game->start(PlayState::getInstance());
        else if( strcmp( argv[1], "mainmenu" ) == 0 )
            game->start(MainmenuState::getInstance());
        else
            game->start(PlayState::getInstance());
    }
    catch (Ogre::Exception& e) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 
        MessageBoxA(NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        std::cerr << "Exception:" << std::endl;
        std::cerr << e.getFullDescription().c_str() << std::endl;
#endif
        return 1;
    }

    delete gameWorld;
    delete game;
    delete physics;
    delete soundManager;

    return 0;
}


/**
    Ensure we run Dungeonhack from the bin directory

    @return 1 if ok, 0 else
*/
int checkWorkingDirectory()
{
    FILE* f;
    if (!(f = fopen("resources.cfg", "r")))
    {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        MessageBoxA(NULL, "Please ensure you are in the bin directory before running the game!", "Wrong directory", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        std::cerr << "Please ensure you are in the bin directory before running the game!" << std::endl;
#endif
        return 0;
    }
    fclose(f);
    return 1;
}
