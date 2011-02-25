/*
 * VisionSystem for VisionSorter
 * Copyright (C) 2010, Ben Axnick
 * Ben Axnick <ben@axnick.com.au>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "BallPlugin.h"
#include "util.h"
#include "lcmtypes/balls_t.h"
#include "lcmtypes/ball_t.h"

#include <osg/ShapeDrawable>
#include <osg/Geode>

#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>

#include <opencv/cv.hpp>
#include <opencv/cxcore.hpp>
#include <opencv/cxmat.hpp>
#include <string.h>
#include <sstream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

using namespace std;

int parseColour(const std::string &colour)
{
    if (colour == string("RED"))
        return BALL_T_RED;
    else if (colour == string("BLUE"))
        return BALL_T_BLUE;
    else
        return -1;
}

void BallCharacteristics::load(std::string paramSet)
{
    std::stringstream ss;
    ss.str(paramSet);
    std::string colourStr;

    ss >> m_name >> m_hue_min >> m_hue_max >> m_sat_min >> m_val_min;
    m_hue_min /= 2;
    m_hue_max /= 2;

    m_sat_min *= 2.55;
    m_val_min *= 2.55;

    m_colour = parseColour(m_name);
    if (m_colour == -1)
    {
        cout << "You entered a colour that does not exist: " << m_name << endl;
        throw exception();
    }
}

void BallSettings::load(const std::string &path)
{
    using boost::property_tree::ptree;
    ptree pt;

    read_ini(path, pt);

    m_transmitRate = pt.get("ball.transmit_rate", 0.5);
    m_bottomAng = pt.get("ball.angle_to_bottom", 270.0);
    m_bottomAng = m_bottomAng * M_PI / 180.;

    m_preBlur = pt.get("ball.pre_blur", 6.);
    m_postBlur = pt.get("ball.post_blur", 4.);
    m_strelSize = pt.get("ball.strel_size", 3.);
    m_pixelThreshold = pt.get("ball.pixel_threshold", 0.03);
    m_houghMinDist = pt.get("ball.hough_min_dist", 70.);
    m_houghParam1 = pt.get("ball.hough_param_1", 145.);
    m_houghParam2 = pt.get("ball.hough_param_2", 15.);
    m_minRadius = pt.get("ball.hough_min_radius", 20.);

    int colour_count = pt.get("ball.colour_count", 0);

    for (int i = 0; i < colour_count; i++)
    {
        stringstream iStr; iStr << i;
        BallCharacteristics ballParamSet;
        std::string colourProp = string("ball.colour/") + iStr.str();
        colourProp = pt.get<std::string>(colourProp.c_str());
        ballParamSet.load(colourProp);
        m_ballParams.push_back(ballParamSet);
    }

    m_weightFactor = pt.get("ball.bottom_weight_factor", 0.5);
    m_confidenceThreshold = pt.get("ball.confidence_threshold", 0.3);
    m_ageThreshold = pt.get("ball.age_threshold", 2.0);
    m_detectTTL = pt.get("ball.ttl", 3.0);
    m_detectHistory = pt.get("ball.history", (uint) 20);
    m_errRadius = pt.get("ball.error", 0.1);
}

BallPlugin::BallPlugin(PluginManager *manager)
{
    this->BasePlugin::Construct(manager, BALL_ID, PRIORITY_MED);
}

bool BallPlugin::Init(PluginManager *manager, const std::string &cfgFile)
{
    m_cfg.load(cfgFile);

    m_sceneGroup = new osg::Group();
    m_tableRef->IncludeInScene(m_sceneGroup);

    if (!m_manager->ShowUi()) return true;

    std::vector<BallCharacteristics>::iterator iter;
    for(iter = m_cfg.m_ballParams.begin(); iter != m_cfg.m_ballParams.end(); iter++)
    {
        BallCharacteristics colourSet = *iter;

        cv::namedWindow (colourSet.m_name);
        cv::waitKey(3);
    }

    return true;
}

void BallPlugin::AnnounceClients(std::vector<Plugin*> &clients)
{
    m_tableRef = TablePlugin::getTableRef(clients);

    if (m_tableRef == NULL)
    {
        cout << "A table reference wasn't found. This is needed for tracking." << endl;
        throw exception();
    }
}

// Derived from parts 3, 4, and 5 of the IHeartRobotics tutorial series at:
// http://www.iheartrobotics.com/search/label/Vision for Robots
std::vector<cv::Vec3f> findCircles(
    const BallSettings &gCfg, const BallCharacteristics &iCfg,
    const cv::Mat &hue, const cv::Mat &sat, const cv::Mat &val, cv::Mat &maskImg)
{
    int significantPixels = 0;
    for(int i = 0; i < maskImg.rows; i++)
    {
      for(int j = 0; j < maskImg.cols; j++)
      {
        if (iCfg.m_hue_min <= iCfg.m_hue_max &&
                (hue.at<unsigned char>(i,j) < iCfg.m_hue_min ||
                hue.at<unsigned char>(i,j) > iCfg.m_hue_max))
        {
            maskImg.at<unsigned char>(i,j) = 0;
        }
        else if (iCfg.m_hue_min > iCfg.m_hue_max &&
            (hue.at<unsigned char>(i,j) < iCfg.m_hue_min &&
            hue.at<unsigned char>(i,j) > iCfg.m_hue_max))
        {
            maskImg.at<unsigned char>(i,j) = 0;
        }
        else if (sat.at<unsigned char>(i,j) < iCfg.m_sat_min)
        {
            maskImg.at<unsigned char>(i,j) = 0;
        }
        else if (val.at<unsigned char>(i,j) < iCfg.m_val_min)
        {
            maskImg.at<unsigned char>(i,j) = 0;
        }
        else
        {
            significantPixels++;
            maskImg.at<unsigned char>(i,j) = 255;
        }
    }
    }

    // Don't waste processing time if nothing to process
    double significantPct = (double) significantPixels / (maskImg.cols * maskImg.rows);
    if (significantPct < gCfg.m_pixelThreshold)
    {
        return std::vector<cv::Vec3f>();
    }

    cv::Size strel_size;
    strel_size.width = gCfg.m_strelSize;
    strel_size.height = gCfg.m_strelSize;
    cv::Mat strel = cv::getStructuringElement(cv::MORPH_ELLIPSE,strel_size);
    cv::morphologyEx(maskImg,maskImg,cv::MORPH_OPEN,strel,cv::Point(-1, -1),3);

    // Convert White on Black to Black on White by inverting the image
    cv::bitwise_not(maskImg, maskImg);
    // Blur the image to improve detection
    cv::GaussianBlur(maskImg,maskImg, cv::Size(9,9), gCfg.m_postBlur, gCfg.m_postBlur );

    // See http://opencv.willowgarage.com/documentation/cpp/feature_detection.html?highlight=hough#HoughCircles
        // The vector circles will hold the position and radius of the detected circles
        std::vector<cv::Vec3f> circles;

        cv::HoughCircles(maskImg, circles, CV_HOUGH_GRADIENT, 1,
                         gCfg.m_houghMinDist,
                         gCfg.m_houghParam1, gCfg.m_houghParam2,
                         gCfg.m_minRadius, maskImg.cols / 4);
    return circles;
}

inline void cvPtToOsg(osg::Vec2d &pt, int width, int height)
{
    pt.x() = 2. * pt.x() / width - 1.;
    pt.y() = 2. * pt.y() / height - 1.;
    pt.y() *= -1;
}

inline void detectPointFromCV(DetectedPoint &dp,
    const cv::Vec3f &circle, double offsetAngle, int width, int height)
{
    dp.m_center = osg::Vec2d(circle[0], circle[1]);
    float radius = circle[2];

    dp.m_offset = osg::Vec2d(radius * cos(offsetAngle), radius * sin(offsetAngle));
    dp.m_offset = dp.m_center - dp.m_offset;

    cvPtToOsg(dp.m_center, width, height);
    cvPtToOsg(dp.m_offset, width, height);
    dp.m_radius = radius / width;
}

void markBallLocations(osg::Group *node, const vector<DetectCluster*> &points, float weightFactor)
{
    node->removeChildren(0, node->getNumChildren());

    vector<DetectCluster*>::const_iterator iter;
    for(iter = points.begin(); iter != points.end(); iter++)
    {
        DetectCluster *dc = *iter;
        DetectedPoint dp = dc->averagedPoint(weightFactor);

        float radius = 5.0f;
        osg::ShapeDrawable *sd = new osg::ShapeDrawable(new osg::Sphere(
                osg::Vec3f(dp.m_real.x(), dp.m_real.y(), radius), radius));

        sd->setColor(osg::Vec4(1, 1, 1, 1));
        osg::Geode* basicShapesGeode = new osg::Geode();

        basicShapesGeode->addDrawable(sd);
        node->addChild(basicShapesGeode);
    }
}

// Derived from part 2 of the IHeartRobotics tutorial series at:
// http://www.iheartrobotics.com/search/label/Vision for Robots
void BallPlugin::IncomingFrame(osgART::GenericVideo* sourceVid, osg::Timer_t now, double elapsed)
{
    m_elapsedTime += elapsed;

    unsigned char imgCpy[sourceVid->getHeight() * sourceVid->getWidth() * 3];
    memcpy(imgCpy, sourceVid->getImage()->data(), sizeof(imgCpy));

    cv::Mat finalImg = cv::Mat(
                sourceVid->getHeight(), sourceVid->getWidth(),
                 CV_8UC3, sourceVid->getImageRaw());

    cv::Mat incImg = cv::Mat(
                sourceVid->getHeight(), sourceVid->getWidth(),
                 CV_8UC3, imgCpy);
    cv::cvtColor(incImg, incImg, CV_RGB2BGR);

    cv::Mat hsvImg = incImg.clone();
    cv::cvtColor (incImg, hsvImg, CV_BGR2HSV);

    if (m_cfg.m_preBlur > 0)
    {
        cv::GaussianBlur(hsvImg,hsvImg, cv::Size(9,9), m_cfg.m_preBlur, m_cfg.m_preBlur);
    }

    cv::Mat hue = cv::Mat::zeros(hsvImg.rows, hsvImg.cols, CV_8U);
    cv::Mat sat = cv::Mat::zeros(hsvImg.rows, hsvImg.cols, CV_8U);
    cv::Mat val = cv::Mat::zeros(hsvImg.rows, hsvImg.cols, CV_8U);


    int from_to[] = { 0,0, 1,1, 2,2};
    cv::Mat img_split[] = { hue, sat, val};
    cv::mixChannels(&hsvImg, 3,img_split,3,from_to,3);


    // Hard coded bottom angle for now
    float offsetAngle = m_cfg.m_bottomAng;


    // DETECTION
    std::vector<DetectedPoint> detected;
    std::vector<BallCharacteristics>::iterator ballIter;
    for(ballIter = m_cfg.m_ballParams.begin(); ballIter != m_cfg.m_ballParams.end(); ballIter++)
    {
        BallCharacteristics colourSet = *ballIter;

        cv::Mat maskImg = cv::Mat::zeros(hsvImg.rows, hsvImg.cols, CV_8U);
        std::vector<cv::Vec3f> circles = findCircles(
                    m_cfg, colourSet, hue, sat, val, maskImg);

        for( size_t i = 0; i < circles.size(); i++ )
        {
            DetectedPoint dp;
            detectPointFromCV(dp, circles[i], offsetAngle, maskImg.cols, maskImg.rows);
            dp.m_colour = colourSet.m_colour;

            if (!m_tableRef->CanHasTracking()->hasVision()) continue;
            dp.m_real = m_tableRef->Surface()->Unproject(dp.m_offset);

            if (m_tableRef->Surface()->IsInBounds(dp.m_real))
            {
                detected.push_back(dp);
            }
        }

        if (m_manager->ShowUi())
        {
            cv::imshow(colourSet.m_name, maskImg);
            cv::waitKey(3);
        }
    }

    // CLUSTERING
    for (uint j = 0; j < detected.size(); j++)
    {
        bool foundMatch = false;
        for(uint i = 0; i < m_clusters.size(); i++)
        {
            if (m_clusters[i].inRange(detected[j], m_cfg.m_weightFactor))
            {
                m_clusters[i].newPoint(detected[j]);
                foundMatch = true;
                break;
            }
        }

        if (!foundMatch)
        {
            DetectCluster newCluster(&m_cfg);
            newCluster.newPoint(detected[j]);
            m_clusters.push_back(newCluster);
        }
    }

    std::vector<DetectCluster> safeClusters;
    for(uint i = 0; i < m_clusters.size(); i++)
    {
        bool alive = m_clusters[i].tick(elapsed);
        if (alive) safeClusters.push_back(m_clusters[i]);
    }

    m_clusters = safeClusters;
    vector<DetectCluster*> viable = viablePoints();
    markBallLocations(m_sceneGroup, viable, m_cfg.m_weightFactor);


    // MESSAGING
    if (m_elapsedTime < m_cfg.m_transmitRate) return;
    m_elapsedTime = fmod(m_elapsedTime, m_cfg.m_transmitRate);

    balls_t message;
    message.info = m_manager->GetInfo(now);

    ball_t indieBalls[viable.size()];
    message.balls_size = viable.size();
    message.balls = indieBalls;

    for(uint i = 0; i < viable.size(); i++)
    {
        DetectCluster* dc = viable[i];
        DetectedPoint dp = dc->averagedPoint(m_cfg.m_weightFactor);

        ball_t ballMessage;

        ballMessage.colour = dp.m_colour;
        ballMessage.position[0] = dp.m_real.x();
        ballMessage.position[1] = dp.m_real.y();
        ballMessage.age = dc->age();
        ballMessage.confidence = dc->confidence();

        indieBalls[i] = ballMessage;
    }

    lcm_t *comms = m_manager->GetComms();
    balls_t_publish(comms, BALL_ID, &message);

}

void BallPlugin::IncludeInScene(osg::Node* child)
{
    // Not required
}

vector<DetectCluster*> BallPlugin::viablePoints()
{
    vector<DetectCluster*> viable;

    vector<DetectCluster>::iterator iter;

    for (iter = m_clusters.begin(); iter != m_clusters.end(); iter++)
    {
        DetectCluster cluster = *iter;

        if (cluster.age() < m_cfg.m_ageThreshold) continue;
        if (cluster.confidence() < m_cfg.m_confidenceThreshold) continue;
        viable.push_back(&(*iter));
    }

    return viable;
}

CamTracker* BallPlugin::CanHasTracking(){ return NULL; }


        DetectCluster::DetectCluster(const BallSettings *settings)
            :m_settings(settings)
        {
            m_ttl = settings->m_detectTTL;
        }

        bool DetectCluster::tick(double time)
        {
            m_age += time;
            m_ttl -= time;
            m_ticks++;

            if (m_ttl > 0) return true;
            else return false;
        }

        bool DetectCluster::inRange(DetectedPoint pt, float weightFactor)
        {
            DetectedPoint avg = averagedPoint(weightFactor);

            osg::Vec2d distance = avg.m_real - pt.m_real;
            if (pt.m_colour != m_members[0].m_colour)
            {
                return false;
            }
            else if (
                    sqrt(
                         distance.x()*distance.x() +
                         distance.y()*distance.y())
                     > m_settings->m_errRadius)
            {
                     return false;
            }
            else
            {
                return true;
            }
        }

        void DetectCluster::newPoint(DetectedPoint pt)
        {
            m_members.push_back(pt);
            m_ttl = m_settings->m_detectTTL;
            m_votes++;

            if (m_members.size() > m_settings->m_detectHistory)
                m_members.erase(m_members.begin());
        }

        DetectedPoint DetectCluster::averagedPoint(float weightBase)
        {
            if (m_members.size() == 1) return m_members[0];

            DetectedPoint avgPoint = m_members[0];
            avgPoint.m_offset = osg::Vec2d(0,0);
            avgPoint.m_center = osg::Vec2d(0,0);
            avgPoint.m_real = osg::Vec2d(0,0);
            avgPoint.m_radius = 0;

            float lowestPoint = m_members[0].m_offset.y();
            float highestPoint = m_members[0].m_offset.y();
            for(uint i = 1; i < m_members.size(); i++)
            {
                if (m_members[i].m_offset.y() < lowestPoint) lowestPoint = m_members[i].m_offset.y();
                else if (m_members[i].m_offset.y() > highestPoint) highestPoint = m_members[i].m_offset.y();
            }

            float height = highestPoint - lowestPoint;

            vector<float> weights;
            float weightSum = 0;
            for(uint i = 0; i < m_members.size(); i++)
            {
                float weighting = weightBase + (1. - weightBase) * (m_members[i].m_offset.y() - highestPoint) / height;
                weights.push_back(weighting);
                weightSum += weighting;
            }

            for(uint i = 0; i < m_members.size(); i++)
            {
                avgPoint.m_center += m_members[i].m_center * weights[i];
                avgPoint.m_offset += m_members[i].m_offset * weights[i];
                avgPoint.m_real += m_members[i].m_real * weights[i];
                avgPoint.m_radius += m_members[i].m_radius * weights[i];
            }

            avgPoint.m_offset /= weightSum;
            avgPoint.m_center /= weightSum;
            avgPoint.m_radius /= weightSum;
            avgPoint.m_real /= weightSum;;

            return avgPoint;
        }

        double DetectCluster::confidence() { return m_votes / m_ticks; }
        double DetectCluster::age() { return m_age; }
