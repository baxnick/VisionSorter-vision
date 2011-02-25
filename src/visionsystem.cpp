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

#include "visionsystem.h"
#include "lcmtypes/info_t.h"

// required
#include <osgART/Foundation>
#include <osgART/VideoManager>
#include <osgART/ARTTransform>
#include <osgART/TrackerManager>
#include <osgART/VideoBackground>
#include <osgProducer/Viewer>

// possibly esoteric
#include <osgCal/Model>
#include <osg/LightModel>
#include <osg/MatrixTransform>

using namespace std;
using namespace osgART;

#define EMPTY_TRACKER "Data/empty.cfg"

osg::ref_ptr<GenericTracker> LoadTracker(GenericVideo *video, const string &cameraDat, const string &trackerCfg)
{
    cout << "LOADING TRACKER DAT: " << cameraDat << " CFG: " << trackerCfg << endl;
    /* load a tracker plugin */
    const char* tracker_plugin_name = getenv("TRACKER");
    if(!tracker_plugin_name) tracker_plugin_name = "osgart_artoolkitplus_tracker";
        osg::ref_ptr<GenericTracker> tracker =
        TrackerManager::createTrackerFromPlugin(tracker_plugin_name);
        if (tracker.valid())
        {
                // access a field within the tracker
                osg::ref_ptr< TypedField<int> > _threshold = reinterpret_cast< TypedField<int>* >(tracker->get("threshold"));
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
                return NULL;
        }



        tracker->init(video->getWidth(), video->getHeight(),trackerCfg.c_str(), cameraDat.c_str());

        return tracker;
}

VisionSystem::VisionSystem(int argc, char **argv)
    : m_argc(argc),
      m_argv(argv),
      m_showUi(false),
      m_iniFile(General::INI_PATH)
{}


osg::ref_ptr<GenericTracker> VisionSystem::ConstructTracker(std::string trackerCfg){
    osg::ref_ptr<GenericTracker> tracker = LoadTracker(m_video.get(), m_cameraDat, trackerCfg);
    m_trackers.push_back(tracker);
    return tracker;
}

osg::Projection* VisionSystem::GetProjection()
{
    return m_projectionMatrix;
}

void VisionSystem::AddScene(osg::Node* node)
{
        node->getOrCreateStateSet()->setRenderBinDetails(5, "RenderBin");
        m_sceneGroup->addChild(node);
}

void VisionSystem::RegisterViewEvent(osgGA::GUIEventHandler *handler)
{
m_viewer.getEventHandlerList().push_back(handler);
}

lcm_t* VisionSystem::GetComms(){
    return m_lcm;
}

info_t VisionSystem::GetInfo(osg::Timer_t stamp)
{
    info_t info;
    info.source = (char*) m_id.c_str();
    info.timestamp = osg::Timer::instance()->delta_m(m_startTime, stamp);

    return info;
}

bool VisionSystem::Init()
{
    if (m_argc < 3 || m_argc > 6)
    {
        cout << m_argc << endl;
        cout << "Args: <camId> <datafile> [width] [height] [iniPath]" << endl;
        return false;
    }

    // INITIALISATION OF OSG STUFF

    m_id = std::string(m_argv[1]);
    m_cameraDat = std::string(m_argv[2]);

    int vidx = -1;
    int vidy = -1;

    if (m_argc >= 5)
    {
        vidx = atoi(m_argv[3]);
        vidy = atoi(m_argv[4]);
        m_showUi = true;
    }

    if (m_argc == 4)
    {
        m_iniFile = std::string(m_argv[3]);
    }
    else if (m_argc == 6)
    {
        m_iniFile = std::string(m_argv[5]);
    }

    osg::setNotifyLevel(osg::INFO);
    osgARTInit(&m_argc, m_argv);
    osg::DisplaySettings::instance()->setMinimumNumStencilBits(8);

    if (m_showUi)
    {
        m_viewer.setUpViewer(osgProducer::Viewer::ESCAPE_SETS_DONE);
        m_viewer.getCullSettings().setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

        m_viewer.getCamera(0)->getRenderSurface()->setWindowRectangle(0,0,vidx,vidy); // viewer accepts W/H arguments from argv
    }

    // load a video plugin
    m_video = VideoManager::createVideoFromPlugin("osgart_artoolkit");
    if (!m_video.valid())
    {
            osg::notify(osg::FATAL) << "Could not initialize video!" << std::endl;
            exit(1); // quit program
    }

    printf("video open....\n");
    m_video->open();

    osg::Light* light = new osg::Light();
    light->setAmbient(osg::Vec4d(0.1, 0.1, 0.1, 1.0));
    light->setDiffuse(osg::Vec4d(1.0, 1.0, 1.0, 1.0));
    light->setLinearAttenuation(0.1);
    light->setQuadraticAttenuation(0.1);
    osg::LightSource * lightsource = new osg::LightSource();
    lightsource->setLight(light);

    // adding the video background
    osg::Group *foregroundGroup = new osg::Group();
    VideoBackground *videoBackground=new VideoBackground(m_video.get());
    videoBackground->setTextureMode(GenericVideoObject::USE_TEXTURE_RECTANGLE);
    videoBackground->init();
    foregroundGroup->addChild(videoBackground);
    foregroundGroup->getOrCreateStateSet()->setRenderBinDetails(2, "RenderBin");

    //create scene to hold all markers
    m_sceneGroup = new osg::Group();
    m_sceneGroup->getOrCreateStateSet()->setRenderBinDetails(5, "RenderBin");

    foregroundGroup->addChild(m_sceneGroup);
    lightsource->setStateSetModes(*(m_sceneGroup->getOrCreateStateSet()), osg::StateAttribute::ON);
    foregroundGroup->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::ON);
    foregroundGroup->addChild(lightsource);
    osg::ref_ptr<GenericTracker> tracker = LoadTracker(m_video.get(), m_cameraDat, std::string(EMPTY_TRACKER));

    if (!tracker)
    {
        cout << "An error occurred while loading the tracker." << endl;
        return false;
    }

    m_projectionMatrix = new osg::Projection(osg::Matrix(tracker->getProjectionMatrix()));
    osg::MatrixTransform* modelViewMatrix = new osg::MatrixTransform();
    modelViewMatrix->addChild(foregroundGroup);
    m_projectionMatrix->addChild(modelViewMatrix);

    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild(m_projectionMatrix);

    m_viewer.setSceneData(root.get());


    // SOMEWHAT LESS INVOLVED INITIALISATION OF LCM STUFF

    m_lcm = lcm_create (NULL);

    if (!m_lcm)
    {
        cout << "Failed during LCM creation" << endl;
        return false;
    }

    return true;
}

bool VisionSystem::LoadPlugin(Plugin *plugin)
{
    cout << "*** Loaded " << plugin->Id() << endl;
    m_plugins.push_back(plugin);
    return true;
}

bool VisionSystem::PrepareForRun()
{
    vector<Plugin*>::iterator plugIt;


    for(plugIt = m_plugins.begin(); plugIt != m_plugins.end(); plugIt++)
    {
        Plugin *plugin = *plugIt;
        plugin->AnnounceClients(m_plugins);
    }

    for(plugIt = m_plugins.begin(); plugIt != m_plugins.end(); plugIt++)
    {
        Plugin *plugin = *plugIt;

        cout << "Preparing plugin " << plugin->Id() << endl;
        bool loaded = plugin->Init(this, m_iniFile);

        if (!loaded)
        {
            cout << "Could not load plugin: " << plugin->Id() << endl;
            return false;
        }
    }

    return true;
}

bool VisionSystem::Done()
{
    if (m_showUi)
        return m_viewer.done();
    else
        return false;
}

void VisionSystem::Run()
{
    if (m_showUi) m_viewer.realize();
    m_video->start();

    osg::Timer_t lastFrameTime = osg::Timer::instance()->tick();
    m_startTime = lastFrameTime;
    while (!Done())
    {

        osg::Timer_t now = osg::Timer::instance()->tick();
        double frameTime = osg::Timer::instance()->delta_s(lastFrameTime,now);
        lastFrameTime = now;

        if (m_showUi) m_viewer.sync();
        m_video->update();

        vector<osg::ref_ptr<GenericTracker> >::iterator trackIt;

        for (trackIt = m_trackers.begin(); trackIt != m_trackers.end(); trackIt++)
        {
            osg::ref_ptr<GenericTracker> tracker = *trackIt;
            tracker->setImage(m_video.get());
            tracker->update();
        }

        vector<Plugin*>::iterator plugIt;

        for(plugIt = m_plugins.begin(); plugIt != m_plugins.end(); plugIt++)
        {
            Plugin *plugin = *plugIt;
            plugin->IncomingFrame(m_video.get(), now, frameTime);
        }

        if (m_showUi)
        {
            m_viewer.update();
            m_viewer.frame();
        }
    }

    if (m_showUi)
    {
        m_viewer.sync();
        m_viewer.cleanup_frame();
        m_viewer.sync();
    }

    m_video->stop();
    m_video->close();
}


bool VisionSystem::ShowUi()
{
    return m_showUi;
}

std::string VisionSystem::CfgPath()
{
    return m_iniFile;
}
