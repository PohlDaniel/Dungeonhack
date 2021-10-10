#ifndef UIMANAGER_H_
#define UIMANAGER_H_

#include <MyGUI_Gui.h>
#include <OgreSingleton.h>

/**
    UI Manager class: abstraction of the GUI framework
*/
class UiManager : public Ogre::Singleton<UiManager>
{
public:
    /**
        Constructor
    */
    UiManager(Ogre::RenderWindow* window);

    /**
        Destructor
    */
    ~UiManager();

    /* Singleton management */
    static const UiManager& getSingleton() { return *ms_Singleton; }
    static UiManager* getSingletonPtr() { return ms_Singleton; }

    /**
        Accessor
    */
    MyGUI::Gui* getGui() { return m_gui; }

    /**
        Initialize the manager
    */
    void initialize();

#if 0
    /**
        Load UIMOD config files (TO BE REMOVED)
    */
    void loadUiMods(const std::string& path);
#endif

    /* Listeners */
    void injectFrameStarted(const Ogre::FrameEvent& evt);
    void injectKeyPress(const OIS::KeyEvent& arg);
    void injectKeyRelease(const OIS::KeyEvent& arg);
    void injectMouseMove(const OIS::MouseEvent& arg);
    void injectMousePress(const OIS::MouseEvent& arg, OIS::MouseButtonID id);
    void injectMouseRelease(const OIS::MouseEvent& arg, OIS::MouseButtonID id);

    /* Pointer management */
    void showPointer() { m_gui->showPointer(); }
    void hidePointer() { m_gui->hidePointer(); }

    /**
        Widget accessor
        \param _name Name of the widget
        \param _throw Throw an exception if not found
        \return The widget
    */
    template <typename T> T* findWidget(const std::string& _name, bool _throw = true)
    {
        return m_gui->findWidget<T>(_name, _throw);
    }

private:
    /**
        Placeholder for UIMOD data
    */
    struct UiModDetails
    {
        std::string m_path;
        std::string m_filename;
        std::string m_name;
        std::string m_description;
        std::string m_author;
        std::string m_version;
        std::string m_targetUiVersion;
        std::string m_resourceGroup;

        Ogre::StringVector m_resources;
        Ogre::StringVector m_fileList;
        Ogre::StringVector m_scriptFileList;
    };
    typedef std::vector<UiModDetails*> UiModList;

#if 0
    /* --------------------------- UIMOD parsing ------------------------------------ */
    void loadModDetails(const FsPath& path);
    void loadUiModFile(const std::string& path);
    UiModDetails* getModDetailsByName(const std::string& name);
    UiModDetails* getModDetailsByPath(const std::string& path);

    void setupModResources(UiModDetails* mod);
    void attachMod(UiModDetails* mod);

    bool getAttribute(TiXmlElement* elem, const std::string& name, std::string& ret);
    /* ------------------------------------------------------------------------------ */
#endif

    UiModList m_uiMods;                     /// List of UIMODs
    Ogre::RenderWindow* m_renderWindow;     /// Render window
    MyGUI::Gui* m_gui;                      /// GUI framework instance
};

#endif
