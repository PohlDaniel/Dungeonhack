#ifndef _GuiFrameListener_H
#define _GuiFrameListener_H

#include <Ogre.h>
#include "UiManager.h"


/**
    Frame listener used for GUI states
*/
class BasicGuiFrameListener : public Ogre::FrameListener
{
public:
    BasicGuiFrameListener(RenderWindow* win, Camera* cam)
    {

    };
};

#endif // _GuiFrameListener_H
