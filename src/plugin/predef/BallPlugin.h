#pragma once

#include <plugin/Plugin.h>
#include "TablePlugin.h"

#include <boost/smart_ptr.hpp>

namespace Ball{
    const std::string CHANNEL("BALL");
};

typedef struct detected_point
{
    osg::Vec2d m_center;
    osg::Vec2d m_offset;
    osg::Vec2d m_real;
    double m_radius;
    int m_colour;
} DetectedPoint;

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
    double m_pixelThreshold;

    double m_weightFactor;
    double m_confidenceThreshold;
    double m_ageThreshold;
    double m_detectTTL;
    int m_detectHistory;
    double m_errRadius;

} BallSettings;

class DetectCluster
{
    public:
        DetectCluster(const BallSettings *settings);
        bool tick(double time);
        bool inRange(DetectedPoint pt, float weightFactor);
        void newPoint(DetectedPoint pt);

        DetectedPoint averagedPoint(float weightBase);
        double confidence();
        double age();
        int colour();
    private:
        const BallSettings *m_settings;
        double m_age;
        double m_ttl;
        double m_ticks;
        double m_votes;
        std::vector<DetectedPoint> m_members;
};

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
    std::vector<DetectCluster*> viablePoints();
    osg::Group *m_sceneGroup;
    BallSettings m_cfg;
    TablePlugin *m_tableRef;
    std::vector<DetectCluster> m_clusters;
};

