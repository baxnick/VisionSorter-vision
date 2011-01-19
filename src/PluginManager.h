#pragma once

#include <osg/Node>
#include <osgGA/GUIEventHandler>
#include <osgART/TrackerManager>
#include <lcm/lcm.h>

#include "info_t.h"

class PluginManager
{
public:
    virtual void RegisterViewEvent (osgGA::GUIEventHandler *handler) = 0;
    virtual void AddScene (osg::Node node) = 0;
    virtual osgART::GenericTracker* ConstructTracker(std::string dataFile) = 0;

    // TODO import LCM
    virtual lcm_t* GetComms() = 0;
    virtual info_t GetInfo() = 0;
};
