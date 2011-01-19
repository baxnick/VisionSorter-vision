/* ========================================================================
* PROJECT: ARToolKitPlus
* ========================================================================
* This work is based on the original ARToolKit developed by
*   Hirokazu Kato
*   Mark Billinghurst
*   HITLab, University of Washington, Seattle
* http://www.hitl.washington.edu/artoolkit/
*
* Copyright of the derived and new portions of this work
*     (C) 2006 Graz University of Technology
*
* This framework is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This framework is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this framework; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
* For further information please contact
*   Dieter Schmalstieg
*   <schmalstieg@icg.tu-graz.ac.at>
*   Graz University of Technology,
*   Institut for Computer Graphics and Vision,
*   Inffeldgasse 16a, 8010 Graz, Austria.
* ========================================================================
** @author   Daniel Wagner
*
* $Id: main.cpp 126 2006-01-26 14:01:41Z daniel $
* @file
 * ======================================================================== */


//
// Simple example to demonstrate multi-marker tracking with ARToolKitPlus
// This sample does not open any graphics window. It just
// loads test images and shows use to use the ARToolKitPlus API.
//

// required
#include <osgCal/Model>
#include <osgART/Foundation>
#include <osgART/VideoManager>
#include <osgART/ARTTransform>
#include <osgART/TrackerManager>
#include <osgART/VideoBackground>
#include <osgProducer/Viewer>

// possibly esoteric
#include <osg/LightModel>
#include <osg/MatrixTransform>

// local
#include "util.h"
#include "CamTracker.h"
#include "Trackables.h"
#include "Plugin.h"
#include "PluginManager.h"

using namespace std;

#if 0
#include <Producer/RenderSurface>
#include <osg/Notify>
#include <osg/Node>
#include <osg/Group>
#include <osg/Switch>
#include <osg/Geode>
#include <osg/Projection>
#include <osg/AutoTransform>
#include <osg/ShapeDrawable>
#include <osg/Geometry>
#include <osg/Image>
#include <osg/LightSource>
#include <osg/Timer>
#include <osg/Version>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgText/Text>
#include <osg/CoordinateSystemNode>
#include <osg/PositionAttitudeTransform>
#include <osg/Projection>
#include <osg/PolygonMode>
#include <osgUtil/SceneView>
#include <osg/Depth>
#include <osg/Stencil>
#endif

class MyMouseEventHandler : public osgGA::GUIEventHandler
    {
    public:
       MyMouseEventHandler(CamTracker *tTracker, CamTracker *cTracker);
       virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&);
       virtual void accept(osgGA::GUIEventHandlerVisitor& v)   { v.visit(*this); };
    private:
        CamTracker *m_cTracker;
        CamTracker *m_tTracker;
};

MyMouseEventHandler::MyMouseEventHandler(CamTracker *tTracker, CamTracker *cTracker)
    :m_cTracker(cTracker),
      m_tTracker(tTracker)
{}

bool MyMouseEventHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&)
{
    int event = ea.getEventType();

    if (event != osgGA::GUIEventAdapter::PUSH) return false;

    TrackableSurface *ts = new TrackableSurface(m_tTracker, osg::Vec4d(0,0,0,1), osg::Vec4d(0,0,1,0));
    TrackableOffsetCube *tc = new TrackableOffsetCube(m_cTracker, osg::Vec3d(20, -15, 238));

    osg::Vec2d realWorldPos = ts->Unproject(osg::Vec2d(ea.getX(), ea.getY()));
    osg::Vec2d cubeScreenPos = tc->ScreenLocationCube();
    osg::Vec2d cubeBaseScreenPos = tc->ScreenLocationBase();
    osg::Vec2d cubeWorldPos = tc->SurfaceLocation(ts);

    cout << "X: " << ea.getX();
    cout << " Y: " << ea.getY() << endl;
    cout << "RX: " << realWorldPos.x();
    cout << " RY: " << realWorldPos.y() << endl;
    cout << "CCX: " << cubeScreenPos.x();
    cout << " CCY: " << cubeScreenPos.y() << endl;
    cout << "CBX: " << cubeBaseScreenPos.x();
    cout << " CBY: " << cubeBaseScreenPos.y() << endl;
    cout << "CPX: " << cubeWorldPos.x();
    cout << " CPY: " << cubeWorldPos.y() << endl;
    cout << "TH: " << ts->GetHeading();
    cout << " CH: " << tc->GetHeading(ts) << endl;
    return true;
}

int main(int argc, char** argv)
{
    osg::setNotifyLevel(osg::INFO);
        osgARTInit(&argc, argv);
        osgProducer::Viewer viewer;
        osg::DisplaySettings::instance()->setMinimumNumStencilBits(8);
        viewer.setUpViewer(osgProducer::Viewer::ESCAPE_SETS_DONE);
        viewer.getCullSettings().setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

        if (argc != 3)
        {
            cout << argc << endl;
            cout << "Args: <width> <height>" << endl;
            exit(1);
        }

        viewer.getCamera(0)->getRenderSurface()->setWindowRectangle(0,0,atoi(argv[1]),atoi(argv[2])); // viewer accepts W/H arguments from argv

        // load a video plugin
        osg::ref_ptr<osgART::GenericVideo> video = osgART::VideoManager::createVideoFromPlugin("osgart_artoolkit");
        if (!video.valid())
        {
                osg::notify(osg::FATAL) << "Could not initialize video!" << std::endl;
                exit(1); // quit program
        }

    /* load a tracker plugin */
    char* tracker_plugin_name = getenv("TRACKER");
    if(!tracker_plugin_name) tracker_plugin_name = "osgart_artoolkitplus_tracker";
        osg::ref_ptr<osgART::GenericTracker> tracker =
        osgART::TrackerManager::createTrackerFromPlugin(tracker_plugin_name);
        if (tracker.valid())
        {
                // access a field within the tracker
                osg::ref_ptr< osgART::TypedField<int> > _threshold = reinterpret_cast< osgART::TypedField<int>* >(tracker->get("threshold"));
                // values can only be accessed through a get()/set() mechanism
        if (_threshold.valid())
        {
                // set the threshold
                _threshold->set(150);
                // check what we actually get //
                osg::notify() << "Field 'threshold' = " << _threshold->get() << std::endl;
        }
        else
        {
                osg::notify() << "Field 'threshold' supported for this tracker" << std::endl;
        }

        }
        else
        {
                std::cerr << "Could not initialize tracker plugin!" << std::endl;
                exit(-1); // quit program
        }

        printf("video open....\n");
        video->open();

        char* marker_file = "Data/table.cfg";
        char* param_file = "Data/iangood2_mod.dat";

        tracker->init(video->getWidth(), video->getHeight(),marker_file, param_file);
        cout << "Loaded " << tracker->getMarkerCount() << " markers.." << endl;
        if (tracker->getMarkerCount() != 2)
        {
            cout << "And this displeases me." << endl;
            exit(1);
        }

        osg::ref_ptr<osgART::Marker> tableMarker = tracker->getMarker(0);
        tableMarker->setActive(true);
        osg::ref_ptr<osg::MatrixTransform> tableTrans = new osgART::ARTTransform(tableMarker.get());

        tableMarker->setRotationalSmoothing(0.3);
        tableMarker->setTranslationalSmoothing(0.3);

        osg::ref_ptr<osgART::Marker> cubeMarker = tracker->getMarker(1);
        cubeMarker->setActive(true);
        osg::ref_ptr<osg::MatrixTransform> cubeTrans = new osgART::ARTTransform(cubeMarker.get());

        osg::Light* light = new osg::Light();
        light->setAmbient(osg::Vec4d(0.1, 0.1, 0.1, 1.0));
        light->setDiffuse(osg::Vec4d(1.0, 1.0, 1.0, 1.0));
        light->setLinearAttenuation(0.1);
        light->setQuadraticAttenuation(0.1);
        osg::LightSource * lightsource = new osg::LightSource();
        lightsource->setLight(light);

        // adding the video background
        osg::Group *foregroundGroup = new osg::Group();
        osgART::VideoBackground *videoBackground=new osgART::VideoBackground(video.get());
        videoBackground->setTextureMode(osgART::GenericVideoObject::USE_TEXTURE_RECTANGLE);
        videoBackground->init();
        foregroundGroup->addChild(videoBackground);
        foregroundGroup->getOrCreateStateSet()->setRenderBinDetails(2, "RenderBin");

        //create scene to hold all markers
        osg::Group *sceneGroup = new osg::Group();
        sceneGroup->getOrCreateStateSet()->setRenderBinDetails(5, "RenderBin");
        foregroundGroup->addChild(tableTrans.get());
        foregroundGroup->addChild(cubeTrans.get());

        cubeTrans->addChild(sceneGroup);

        foregroundGroup->addChild(lightsource);
        lightsource->setStateSetModes(*(foregroundGroup->getOrCreateStateSet()), osg::StateAttribute::ON);

        float boxSizeX = 90.0f;
        float boxSizeY = 30.0f;
        float boxSizeZ = 5.0f;
        osg::ShapeDrawable *sd = new osg::ShapeDrawable(new osg::Box(osg::Vec3(0, 0, 0), boxSizeX,boxSizeY,boxSizeZ));
        sd->setColor(osg::Vec4(0, 0, 1, 1));

        osg::Geode* basicShapesGeode = new osg::Geode();
        osg::MatrixTransform* transUp =
                new osg::MatrixTransform(osg::Matrix::translate(0, 0, boxSizeZ / 2 + 30.0f));
        basicShapesGeode->addDrawable(sd);
        sceneGroup->addChild(transUp);
        transUp->addChild(basicShapesGeode);

        sceneGroup->addChild(lightsource);
        sceneGroup->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::ON);

        osg::Projection* projectionMatrix = new osg::Projection(osg::Matrix(tracker->getProjectionMatrix()));
        osg::MatrixTransform* modelViewMatrix = new osg::MatrixTransform();
        modelViewMatrix->addChild(foregroundGroup);
        projectionMatrix->addChild(modelViewMatrix);

        osg::ref_ptr<osg::Group> root = new osg::Group;
        root->addChild(projectionMatrix);

        CamTracker *tableTrack = new CamTracker(projectionMatrix, tableMarker.get());
        CamTracker *cubeTrack = new CamTracker(projectionMatrix, cubeMarker.get());
        MyMouseEventHandler* meHandler = new MyMouseEventHandler(tableTrack, cubeTrack);
        viewer.getEventHandlerList().push_back(meHandler);

        viewer.setSceneData(root.get());
        viewer.realize();
        video->start();

        osg::Timer_t lastFrameTime = osg::Timer::instance()->tick();
        osg::Timer_t lastValidTime = osg::Timer::instance()->tick();
        while (!viewer.done())
        {

            osg::Timer_t now = osg::Timer::instance()->tick();
            double frameTime = osg::Timer::instance()->delta_s(lastFrameTime,now);
            lastFrameTime = now;

            viewer.sync();
            video->update();
            tracker->setImage(video.get());
            tracker->update();
            viewer.update();
            viewer.frame();
        }

        viewer.sync();
        viewer.cleanup_frame();
        viewer.sync();
        video->stop();
        video->close();
}
