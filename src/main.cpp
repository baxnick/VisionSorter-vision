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
#include "Plugin.h"
#include "PluginManager.h"
#include "visionsystem.h"
#include "TablePlugin.h"
#include "CubePlugin.h"

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

/*
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
*/

int main(int argc, char** argv)
{
    VisionSystem vs = VisionSystem(string("CAM0"), argc, argv);

    if (!vs.Init())
    {
        cout << "Failed to initialise plugin manager" << endl;
        exit(1);
    }

    TablePlugin *table = new TablePlugin(&vs);
    vs.LoadPlugin(table);


    std::vector < boost::shared_ptr<CubePlugin> > cubes = CubePlugin::LoadPlugins(&vs, string(argv[1]));
    std::vector < boost::shared_ptr<CubePlugin> >::iterator iter;

    for (iter = cubes.begin(); iter != cubes.end(); iter++)
    {
        boost::shared_ptr<CubePlugin> cube = *iter;
        vs.LoadPlugin(cube.get());
    }


    if (!vs.PrepareForRun())
    {
        cout << "Failed to initialise a plugin.." << endl;
        exit(1);
    }

    vs.Run();

/*
        CamTracker *tableTrack = new CamTracker(projectionMatrix, tableMarker.get());
        CamTracker *cubeTrack = new CamTracker(projectionMatrix, cubeMarker.get());
        MyMouseEventHandler* meHandler = new MyMouseEventHandler(tableTrack, cubeTrack);
        viewer.getEventHandlerList().push_back(meHandler);

*/
}
