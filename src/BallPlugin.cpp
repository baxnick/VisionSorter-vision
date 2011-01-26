#include "BallPlugin.h"
#include "util.h"

#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>

#include <opencv/cv.hpp>
#include <opencv/cxcore.hpp>
#include <opencv/cxmat.hpp>
#include <string.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

using namespace std;

typedef struct ball_characteristics
{

} BallCharacteristics;

void BallSettings::load(const std::string &path)
{
    using boost::property_tree::ptree;
    ptree pt;

    read_ini(path, pt);

    m_bottomAng = pt.get("ball.angle_to_bottom", 270.0);
    m_bottomAng = m_bottomAng * M_PI / 180;
}

BallPlugin::BallPlugin(PluginManager *manager)
{
    this->BasePlugin::Construct(manager, BALL_ID, PRIORITY_MED);
}

bool BallPlugin::Init(PluginManager *manager, const std::string &cfgFile)
{
    m_cfg.load(cfgFile);


    cv::namedWindow ("bin", 1);
    cv::waitKey(3);


    cv::namedWindow ("out", 1);
    cv::waitKey(3);
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

void BallPlugin::IncomingFrame(osgART::GenericVideo* sourceVid, osg::Timer_t now, double elapsed)
{
    unsigned char imgCpy[sourceVid->getHeight() * sourceVid->getWidth() * 3];
    memcpy(imgCpy, sourceVid->getImage()->data(), sizeof(imgCpy));

    cv::Mat incImg = cv::Mat(
                sourceVid->getHeight(), sourceVid->getWidth(),
                 CV_8UC3, imgCpy);
    cv::cvtColor(incImg, incImg, CV_RGB2BGR);


    cv::Mat finalImg = cv::Mat(
                sourceVid->getHeight(), sourceVid->getWidth(),
                 CV_8UC3, sourceVid->getImageRaw());
    cv::Mat outImg = incImg.clone();
    cv::Mat hsvImg = incImg.clone();
    cv::cvtColor (incImg, hsvImg, CV_BGR2HSV);
    cv::GaussianBlur(hsvImg,hsvImg, cv::Size(9,9), 6, 6 );

    cv::Mat hue = cv::Mat::zeros(hsvImg.rows, hsvImg.cols, CV_8U);
    cv::Mat sat = cv::Mat::zeros(hsvImg.rows, hsvImg.cols, CV_8U);
    cv::Mat val = cv::Mat::zeros(hsvImg.rows, hsvImg.cols, CV_8U);

    cv::Mat maskImg = cv::Mat::zeros(hsvImg.rows, hsvImg.cols, CV_8U);

    int from_to[] = { 0,0, 1,1, 2,2};
    cv::Mat img_split[] = { hue, sat, val};
    cv::mixChannels(&hsvImg, 3,img_split,3,from_to,3);

    for(int i = 0; i < outImg.rows; i++)
    {
      for(int j = 0; j < outImg.cols; j++)
      {
        // The output pixel is white if the input pixel
        // hue is orange and saturation is reasonable
        if((hue.at<unsigned char>(i,j) < 21 ||
           hue.at<unsigned char>(i,j) > 171) &&
           sat.at<unsigned char>(i,j) > 140 &&
           val.at<unsigned char>(i,j) > 40) {
          maskImg.at<unsigned char>(i,j) = 255;
        } else {
          maskImg.at<unsigned char>(i,j) = 0;
          // Clear pixel blue output channel
          outImg.at<unsigned char>(i,j*3+0) = 0;
          // Clear pixel green output channel
          outImg.at<unsigned char>(i,j*3+1) = 0;
          // Clear pixel red output channel
          outImg.at<unsigned char>(i,j*3+2) = 0;
        }
      }
    }

    cv::Size strel_size;
    strel_size.width = 4;
    strel_size.height = 4;
    cv::Mat strel = cv::getStructuringElement(cv::MORPH_ELLIPSE,strel_size);
    cv::morphologyEx(maskImg,maskImg,cv::MORPH_OPEN,strel,cv::Point(-1, -1),3);

    // Convert White on Black to Black on White by inverting the image
    cv::bitwise_not(maskImg, maskImg);
    // Blur the image to improve detection
    cv::GaussianBlur(maskImg,maskImg, cv::Size(9,9), 4, 4 );

    // See http://opencv.willowgarage.com/documentation/cpp/feature_detection.html?highlight=hough#HoughCircles
        // The vector circles will hold the position and radius of the detected circles
        std::vector<cv::Vec3f> circles;
        std::vector<osg::Vec2d> realPositions;

        // Detect circles That have a radius between 20 and 400 that are a minimum of 70 pixels apart
        cv::HoughCircles(maskImg, circles, CV_HOUGH_GRADIENT, 1, 70, 140, 15, 20, outImg.cols / 4);

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
                 double bankingAng = m_tableRef->CanHasTracking()->FindBanking() * M_PI / 180.;
                 offset = cv::Point(radius * cos(bankingAng), radius * sin(bankingAng));
             }

             offset = center - offset;
             realPositions.push_back(osg::Vec2d(offset.x, offset.y));

             // draw the circle outline
             cv::circle( finalImg, center, radius+1, cv::Scalar(0,0,255), 2, 8, 0 );
             // draw the circle "contact point"
             cv::circle( finalImg, offset, 3, cv::Scalar(0,255,0), -1, 8, 0 );
        }

    cv::imshow("bin", maskImg);
    cv::imshow("out", outImg);
    cv::waitKey(3);
}

void BallPlugin::IncludeInScene(osg::Node* child)
{
    // TODO
}

CamTracker* BallPlugin::CanHasTracking(){ return NULL; }
