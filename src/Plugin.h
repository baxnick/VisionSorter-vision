#pragma once

#include <osgART/VideoManager>
#include <osg/Timer>
#include <osg/Node>
#include <osg/Vec2>

#include <string>
#include <vector>

#include "PluginManager.h"
#include "CamTracker.h"

class Plugin
{
public:
    virtual std::string Id() = 0;
    virtual int Priority() = 0;

    virtual bool Init(PluginManager *manager, int argc, char** argv) = 0;
    virtual void AnnounceClients(std::vector<Plugin> clients) = 0;

    virtual void IncomingFrame(osgART::GenericVideo* sourceVid, osg::Timer_t now, double elapsed) = 0;
    virtual void IncludeInScene(osg::Node* child) = 0;

    virtual osg::Vec2 WhereAmI(CamTracker* cam) = 0;
};

class BasePlugin : public Plugin
{
    public:
        virtual std::string Id();
        virtual int Priority();
        BasePlugin(PluginManager *manager, std::string id, int priority);
    protected:
        PluginManager *m_manager;
    private:
        std::string m_id;
        int m_priority;
};
