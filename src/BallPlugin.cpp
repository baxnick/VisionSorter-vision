#include "Plugin.h"
#include "BallPlugin.h"
#include "util.h"
#include "balls_t.h"
#include "ball_t.h"

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
}

BallPlugin::BallPlugin(PluginManager *manager)
{
    this->BasePlugin::Construct(manager, BALL_ID, PRIORITY_MED);
}

bool BallPlugin::Init(PluginManager *manager, const std::string &cfgFile)
{
    m_cfg.load(cfgFile);

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
    m_tableRef = getTableRef(clients);

    if (m_tableRef == NULL)
    {
        cout << "A table reference wasn't found. This is needed for tracking." << endl;
        throw exception();
    }
}

std::vector<cv::Vec3f> findCircles(
    const BallSettings &gCfg, const BallCharacteristics &iCfg,
    const cv::Mat &hue, const cv::Mat &sat, const cv::Mat &val, cv::Mat &maskImg)
{
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
            maskImg.at<unsigned char>(i,j) = 255;
        }
    }
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

        // Detect circles That have a radius between 20 and 400 that are a minimum of 70 pixels apart
        cv::HoughCircles(maskImg, circles, CV_HOUGH_GRADIENT, 1,
                         gCfg.m_houghMinDist,
                         gCfg.m_houghParam1, gCfg.m_houghParam2,
                         gCfg.m_minRadius, maskImg.cols / 4);
    return circles;
}

void BallPlugin::IncomingFrame(osgART::GenericVideo* sourceVid, osg::Timer_t now, double elapsed)
{
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

    cv::GaussianBlur(hsvImg,hsvImg, cv::Size(9,9), m_cfg.m_preBlur, m_cfg.m_preBlur);

    cv::Mat hue = cv::Mat::zeros(hsvImg.rows, hsvImg.cols, CV_8U);
    cv::Mat sat = cv::Mat::zeros(hsvImg.rows, hsvImg.cols, CV_8U);
    cv::Mat val = cv::Mat::zeros(hsvImg.rows, hsvImg.cols, CV_8U);


    int from_to[] = { 0,0, 1,1, 2,2};
    cv::Mat img_split[] = { hue, sat, val};
    cv::mixChannels(&hsvImg, 3,img_split,3,from_to,3);

    std::vector<osg::Vec3d> realPositions;
    std::vector<BallCharacteristics>::iterator ballIter;
    for(ballIter = m_cfg.m_ballParams.begin(); ballIter != m_cfg.m_ballParams.end(); ballIter++)
    {
        BallCharacteristics colourSet = *ballIter;

        cv::Mat maskImg = cv::Mat::zeros(hsvImg.rows, hsvImg.cols, CV_8U);
        std::vector<cv::Vec3f> circles = findCircles(
                    m_cfg, colourSet, hue, sat, val, maskImg);

        for( size_t i = 0; i < circles.size(); i++ )
        {
             // round the floats to an int
             cv::Point center(circles[i][0], circles[i][1]);
             float radius = circles[i][2];

             cv::Point offset;
             if (!m_tableRef->CanHasTracking()->hasVision())
                 offset = cv::Point(radius * cos(m_cfg.m_bottomAng), radius * sin(m_cfg.m_bottomAng));
             else
             {
                 double ang = (270 - m_tableRef->CanHasTracking()->FindAttitude()) * M_PI / 180.;
                 offset = cv::Point(radius * cos(ang), radius * sin(ang));
             }

             offset = center - offset;

             // draw the circle outline
             cv::circle( finalImg, center, radius+1, cv::Scalar(0,0,255), 2, 8, 0 );
             // draw the circle "contact point"
             cv::circle( finalImg, offset, 3, cv::Scalar(0,255,0), -1, 8, 0 );


             // Now calculate the real world position
             if (!m_tableRef->CanHasTracking()->hasVision()) continue;

             osg::Vec2d realPos(offset.x, offset.y);

             realPos.x() = 2. * offset.x / incImg.cols - 1.;
             realPos.y() = 2. * offset.y / incImg.rows - 1.;
             realPos.y() *= -1;

             realPos = m_tableRef->Surface()->Unproject(realPos);
             if (m_tableRef->Surface()->IsInBounds(realPos))
             {
                // hackish way to keep track of colour
                osg::Vec3d realPosWithColour = osg::Vec3d(realPos, colourSet.m_colour);
                realPositions.push_back(realPosWithColour);
             }
        }

        cv::imshow(colourSet.m_name, maskImg);
        cv::waitKey(3);
    }

    if (realPositions.size() == 0) return;

    m_elapsedTime += elapsed;

    if (m_elapsedTime < m_cfg.m_transmitRate) return;

    m_elapsedTime = fmod(m_elapsedTime, m_cfg.m_transmitRate);

    balls_t message;
    message.info = m_manager->GetInfo(now);

    ball_t indieBalls[realPositions.size()];

    int i = 0;
    vector<osg::Vec3d>::iterator posIter;
    for (posIter = realPositions.begin(); posIter != realPositions.end(); posIter++, i++)
    {
        osg::Vec3d pos = *posIter;

        ball_t ballMessage;

        ballMessage.colour = pos.z();
        ballMessage.position[0] = pos.x();
        ballMessage.position[1] = pos.y();

        indieBalls[i] = ballMessage;
    }

    message.balls_size = realPositions.size();
    message.balls = indieBalls;

    lcm_t *comms = m_manager->GetComms();
    balls_t_publish(comms, BALL_ID, &message);
}

void BallPlugin::IncludeInScene(osg::Node* child)
{
    // TODO
}

CamTracker* BallPlugin::CanHasTracking(){ return NULL; }
