#pragma once

#include <osg/Group>
#include <osg/Node>
#include <osg/Timer>
#include <osgGA/GUIEventHandler>
#include <osgART/TrackerManager>
#include <lcm/lcm.h>

#include "info_t.h"

class PluginManager
{
public:
    virtual void RegisterViewEvent (osgGA::GUIEventHandler *handler) = 0;
    virtual void AddScene (osg::Node *node) = 0;
    virtual osg::ref_ptr<osgART::GenericTracker> ConstructTracker(std::string trackerCfg) = 0;
    virtual osg::Projection* GetProjection() = 0;

    // TODO import LCM
    virtual lcm_t* GetComms() = 0;
    virtual info_t GetInfo(osg::Timer_t stamp) = 0;

    virtual ~PluginManager(){};
};
