#ifndef DUNGEONHACK_H_
#define DUNGEONHACK_H_

// This is DH's precompiled header, only put static includes here. If a file changes a lot and is included here,
// it defeats the point of the PCH, which is to speed up compile time. Not sure how PCH's work under GCC, but
// they do exist in some form or another

#pragma message("Compiling DungeonHack.h - this should happen just once.\n")

// Precompile the Ogre header
#include <Ogre.h>

// Precompile MyGUI
#include <MyGUI.h>

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#   if _MSC_VER > 1200
        // Get rid of some warnings
#       pragma warning(disable: 4482)
#       pragma warning(disable: 4996)

        // Precompile the windows header
#       define WIN32_LEAN_AND_MEAN
#       include <windows.h>

#   endif
#endif

// Other std libs
#include <string>

#endif
