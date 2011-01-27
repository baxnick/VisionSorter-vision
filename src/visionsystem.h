#pragma once

#include "PluginManager.h"
#include "Plugin.h"

#include <osgProducer/Viewer>

namespace General
{
    const std::string INI_PATH("Data/vision.ini");
}

class VisionSystem : public PluginManager
{
public:
    VisionSystem(int argc, char** argv);
    virtual ~VisionSystem(){};
    bool Init();
    bool LoadPlugin(Plugin *plugin);
    bool PrepareForRun();
    void Run();

    std::string CfgPath();
    virtual bool ShowUi();
    void RegisterViewEvent (osgGA::GUIEventHandler *handler);
    void AddScene (osg::Node *node);
    osg::ref_ptr<osgART::GenericTracker> ConstructTracker(std::string trackerCfg);
    osg::Projection* GetProjection();

    // TODO import LCM
    lcm_t* GetComms();
    info_t GetInfo(osg::Timer_t stamp);

private:
    bool m_showUi;
    int m_argc;
    char** m_argv;
    std::string m_iniFile;
    std::string m_cameraDat;
    std::string m_id;
    std::vector<Plugin*> m_plugins;
    std::vector<osg::ref_ptr<osgART::GenericTracker> > m_trackers;

    osg::Timer_t m_startTime;
    osg::Projection* m_projectionMatrix;
    osgProducer::Viewer m_viewer;
    osg::Group* m_sceneGroup;
    osg::ref_ptr<osgART::GenericVideo> m_video;
    lcm_t* m_lcm;

    bool Done();
};
