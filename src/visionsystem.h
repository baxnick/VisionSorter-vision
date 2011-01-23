#pragma once

#include "PluginManager.h"
#include "Plugin.h"

#include <osgProducer/Viewer>

class VisionSystem : public PluginManager
{
public:
    VisionSystem(std::string id, int argc, char** argv);
    virtual ~VisionSystem(){};
    bool Init();
    bool LoadPlugin(Plugin *plugin);
    bool PrepareForRun();
    void Run();


    void RegisterViewEvent (osgGA::GUIEventHandler *handler);
    void AddScene (osg::Node *node);
    osg::ref_ptr<osgART::GenericTracker> ConstructTracker(std::string trackerCfg);
    osg::Projection* GetProjection();

    // TODO import LCM
    lcm_t* GetComms();
    info_t GetInfo(osg::Timer_t stamp);

private:
    int m_argc;
    char** m_argv;
    std::string m_cfgFile;
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
};
