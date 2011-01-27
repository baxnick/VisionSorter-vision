#pragma once

#include "Plugin.h"
#include "TablePlugin.h"

namespace Ball{
    const std::string CHANNEL("BALL");
};

typedef struct ball_characteristics
{
    void load (std::string paramSet);
    std::string m_name;
    int m_colour;
    int m_hue_min;
    int m_hue_max;
    int m_sat_min;
    int m_val_min;
} BallCharacteristics;

typedef struct ball_settings
{
    void load(const std::string &filename);
    double m_bottomAng;
    double m_transmitRate;

    std::vector<BallCharacteristics> m_ballParams;

    double m_preBlur;
    double m_postBlur;
    double m_strelSize;
    double m_houghMinDist;
    double m_houghParam1;
    double m_houghParam2;
    double m_minRadius;
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

