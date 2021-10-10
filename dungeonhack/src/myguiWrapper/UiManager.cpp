#include "DungeonHack.h"
#include "UiManager.h"


template<> UiManager* Ogre::Singleton<UiManager>::ms_Singleton = 0;

UiManager::UiManager(Ogre::RenderWindow* window)
    : m_renderWindow(window),
    m_gui(0)
{
}

UiManager::~UiManager()
{
    if(m_gui)
    {
        m_gui->shutdown();
        delete m_gui;
    }
}

void UiManager::initialize()
{
    m_gui = new MyGUI::Gui();
    m_gui->initialise(m_renderWindow, "core.xml");
}

#if 0
void UiManager::loadUiMods(const std::string& path)
{
    m_uiMods.clear();

    FsPath uiPath(path);
    directory_iterator dirIter(uiPath);
    directory_iterator end;
    for(; dirIter != end; dirIter++)
    {
        if(is_directory((*dirIter).status()))
        {
            loadModDetails((*dirIter));
        }
    }

    UiModDetails* core = getModDetailsByName("Core");
    if(!core)
    {
        throw std::runtime_error("Could not load core UI mode");
    }

    setupModResources(core);
    attachMod(core);

    UiModList::iterator iter = m_uiMods.begin();
    for(; iter != m_uiMods.end(); iter++)
    {
        if((*iter)->m_name == "Core")
        {
            continue;
        }

        setupModResources((*iter));
        attachMod((*iter));
    }

    m_scriptEngine->bindDeclaredEvents(m_gui);
}
#endif

void UiManager::injectFrameStarted(const Ogre::FrameEvent& evt)
{
    if(m_gui)
    {
        m_gui->injectFrameEntered(evt.timeSinceLastFrame);
    }
}

void UiManager::injectKeyPress(const OIS::KeyEvent& arg)
{
    if(m_gui)
    {
        m_gui->injectKeyPress(arg);
    }
}

void UiManager::injectKeyRelease(const OIS::KeyEvent& arg)
{
    if(m_gui)
    {
        m_gui->injectKeyRelease(arg);
    }
}
void UiManager::injectMouseMove(const OIS::MouseEvent& arg)
{
    if(m_gui)
    {
        m_gui->injectMouseMove(arg);
    }
}

void UiManager::injectMousePress(const OIS::MouseEvent& arg, OIS::MouseButtonID id)
{
    if(m_gui)
    {
        m_gui->injectMousePress(arg, id);
    }
}

void UiManager::injectMouseRelease(const OIS::MouseEvent& arg, OIS::MouseButtonID id)
{
    if(m_gui)
    {
        m_gui->injectMouseRelease(arg, id);
    }
}

#if 0
void UiManager::loadModDetails(const FsPath& path)
{
    directory_iterator dirIter(path);
    directory_iterator end;
    for(; dirIter != end; dirIter++)
    {
        if(is_regular((*dirIter).status()))
        {
            if(extension((*dirIter)) == ".uimod")
            {
                UiModDetails* mod = getModDetailsByPath((*dirIter).string());
                if(mod != NULL)
                {
                    return;
                }

                loadUiModFile((*dirIter).string());
            }
        }
    }
}

void UiManager::loadUiModFile(const std::string& path)
{
    UiModDetails* mod = NULL;
    try
    {
        TiXmlDocument doc(path);
        doc.LoadFile();

        mod = new UiModDetails();
        FsPath modPath(path);
        mod->m_path = modPath.branch_path().string();
        mod->m_filename = modPath.leaf();

        TiXmlElement* root = doc.RootElement();
        if(!getAttribute(root, "name", mod->m_name))
        {
            throw std::runtime_error("UI mod must define 'name' attribute");
        }
        if(!getAttribute(root, "description", mod->m_description))
        {
            throw std::runtime_error("UI mod must define 'description' attribute");
        }
        if(!getAttribute(root, "author", mod->m_author))
        {
            throw std::runtime_error("UI mod must define 'author' attribute");
        }
        if(!getAttribute(root, "version", mod->m_version))
        {
            throw std::runtime_error("UI mod must define 'version' attribute");
        }
        if(!getAttribute(root, "uiTarget", mod->m_targetUiVersion))
        {
            throw std::runtime_error("UI mod must define 'uiTarget' attribute");
        }
        if(!getAttribute(root, "resourceGroup", mod->m_resourceGroup))
        {
            throw std::runtime_error("UI mod must define 'resourceGroup' attribute");
        }

        TiXmlElement* resources = root->FirstChildElement("Resources");
        if(resources != NULL)
        {
            TiXmlElement* resPath = resources->FirstChildElement("Path");
            std::string pathValue = "";
            while(resPath != NULL)
            {
                if(getAttribute(resPath, "value", pathValue))
                {
                    mod->m_resources.push_back(pathValue);
                }
                resPath = resPath->NextSiblingElement("Path");
            }
        }

        TiXmlElement* files = root->FirstChildElement("Files");
        if(files != NULL)
        {
            TiXmlElement* file = files->FirstChildElement("File");
            std::string fileName = "";
            while(file != NULL)
            {
                if(getAttribute(file, "name", fileName))
                {
                    mod->m_fileList.push_back(fileName);
                }
                file = file->NextSiblingElement("File");
            }
        }

        TiXmlElement* scripts = root->FirstChildElement("Scripts");
        if(scripts != NULL)
        {
            TiXmlElement* script = scripts->FirstChildElement("Script");
            std::string scriptFile = "";
            while(script != NULL)
            {
                if(getAttribute(script, "file", scriptFile))
                {
                    mod->m_scriptFileList.push_back(scriptFile);
                }
                script = script->NextSiblingElement("Script");
            }
        }

        m_uiMods.push_back(mod);
    }
    catch(...)
    {
        if(mod)
        {
            delete mod;
        }
    }
}

UiManager::UiModDetails* UiManager::getModDetailsByName(const std::string& name)
{
    UiModList::iterator iter = m_uiMods.begin();
    for(; iter != m_uiMods.end(); iter++)
    {
        if((*iter)->m_name == name)
        {
            return (*iter);
        }
    }

    return NULL;
}

UiManager::UiModDetails* UiManager::getModDetailsByPath(const std::string& path)
{
    UiModList::iterator iter = m_uiMods.begin();
    for(; iter != m_uiMods.end(); iter++)
    {
        FsPath modPath((*iter)->m_path);
        if(equivalent(modPath / (*iter)->m_filename, FsPath(path)))
        {
            return (*iter);
        }
    }

    return NULL;
}

void UiManager::setupModResources(UiModDetails* mod)
{
    Ogre::ResourceGroupManager* resGroupMgr = Ogre::ResourceGroupManager::getSingletonPtr();
    resGroupMgr->addResourceLocation(mod->m_path, "FileSystem", "UI", false);

    Ogre::StringVector::iterator iter = mod->m_resources.begin();
    for(; iter != mod->m_resources.end(); iter++)
    {
        resGroupMgr->addResourceLocation((FsPath(mod->m_path) / (*iter)).string(), "FileSystem", "UI", false);
    }
}

void UiManager::attachMod(UiModDetails* mod)
{
    Ogre::StringVector::iterator iter = mod->m_fileList.begin();
    for(; iter != mod->m_fileList.end(); iter++)
    {
        m_gui->load((*iter), "UI");
    }

    iter = mod->m_scriptFileList.begin();
    for(; iter != mod->m_scriptFileList.end(); iter++)
    {
        //(FsPath(mod->m_path) / (*iter)).string().c_str()
        m_scriptEngine->loadScript((FsPath(mod->m_path) / (*iter)).string());
    }
}

bool UiManager::getAttribute(TiXmlElement* elem, const std::string& name, std::string& ret)
{
    ret = "";
    if(!elem)
    {
        return false;
    }

    const char* attr = elem->Attribute(name);
    if(!attr)
    {
        return false;
    }

    ret = attr;
    return true;
}
#endif
