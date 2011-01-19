#pragma once

#include "PluginManager.h"
#include "Plugin.h"

class VisionSystem : public PluginManager
{
public:
    VisionSystem(std::string id, int argc, char** argv);
    bool Init();
    bool LoadPlugin(Plugin *plugin);
    bool PrepareForRun();
    void Run();


    void RegisterViewEvent (osgGA::GUIEventHandler *handler);
    void AddScene (osg::Node node);
    osgART::GenericTracker* ConstructTracker(std::string dataFile);

    // TODO import LCM
    lcm_t* GetComms();
    info_t GetInfo();

private:
    int m_argc;
    char** m_argv;
    std::string m_id;
    std::vector<Plugin> plugins;
};
