#pragma once

#include "Plugin.h"
#include "TablePlugin.h"

namespace Ball{
    const std::string CHANNEL("BALL");
};

typedef struct ball_settings
{
    void load(const std::string &filename);
    double m_bottomAng;
} BallSettings;

class BallPlugin : public BasePlugin
{
public:
    BallPlugin(PluginManager *manager);
    bool Init(PluginManager *manager, const std::string &cfgFile);
    void AnnounceClients(std::vector<Plugin*> &clients);

    void IncomingFrame(osgART::GenericVideo* sourceVid, osg::Timer_t now, double elapsed);
    void IncludeInScene(osg::Node* child);

    CamTracker* CanHasTracking();
private:
    osg::Group *m_sceneGroup;
    BallSettings m_cfg;
    TablePlugin *m_tableRef;
};

