#ifndef _DH_GAMEPREREQS_H
#define _DH_GAMEPREREQS_H

#include <Ogre.h>
#include <iostream>

/// We don't have "NULL" under Linux by default.
#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
    #ifndef NULL
    #define NULL 0
    #endif
#endif

/// Darn STL stuff...
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 && !defined(__GNUC__)
    #include <hash_map>

    typedef stdext::hash_compare<int,std::less<int>> DH_intHash;
    typedef stdext::hash_compare<Ogre::String, std::less<Ogre::String>> DH_StringHash;
#else
    #define stdext __gnu_cxx
    #include <ext/hash_map>

    typedef __gnu_cxx::hash<int> DH_intHash;
    typedef __gnu_cxx::hash<Ogre::String> DH_StringHash;
#endif

// Defining itoa for gcc
#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
    /**
     * C++ version char* style "itoa":
     */
    char* itoa( int value, char* result, int base );

    /**
     * C++ version std::string style "itoa":
     */
    std::string itoa( int value, int base );
#endif

namespace Game
{
    /// You may add here forward declarations too!
    class Item;
    class CollisionObject;
    class VisualRepresentation;
    class GameEntity;
    class GameState;
    class Location;
    class GameActor;
    class WeaponState;

    struct ActorStats;
    struct ActorStatus;
    struct EquippedItems;

    enum GeoDirection
    {
        North = 0,
        South,
        West,
        East
        /// Add some more if you need/like, like NW, SE etc...
    };

    /// More to come ;) ..
}

#endif
