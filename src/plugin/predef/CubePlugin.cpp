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

#include "CubePlugin.h"
#include "lcmtypes/cube_t.h"
#include "util.h"

#include <osg/ShapeDrawable>
#include <osg/Geode>
#include <osg/Matrix>
#include <osg/MatrixTransform>
#include <osgART/ARTTransform>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

void DrawHeadingTriangle(osg::Group *scene, double offset, double radius, double length)
{
   osg::ShapeDrawable *sd = new osg::ShapeDrawable(new osg::Cone(
            osg::Vec3d(0., 0., 0.), radius, length));

   sd->setColor(osg::Vec4(1, 1, 0, 1));
   osg::Geode* basicShapesGeode = new osg::Geode();

   osg::Matrix rotate = osg::Matrixd::rotate(-M_PI_2, 1, 0, 0);
   osg::Matrix translate = osg::Matrixd::translate(0, 0, offset);

   osg::MatrixTransform *position = new osg::MatrixTransform(rotate * translate);
   basicShapesGeode->addDrawable(sd);
   scene->addChild(position);
   position->addChild(basicShapesGeode);
}

void CubeSettings::load(const std::string &path)
{
   using boost::property_tree::ptree;
   ptree pt;

   read_ini(path, pt);

   m_CubesDir = pt.get("cube.cube_path", std::string("Data/Cube"));
   m_rotateSmoothing = pt.get("cube.rotate_smoothing", 0.3);
   m_translateSmoothing = pt.get("cube.translate_smoothing", 0.3);
   m_validThreshold = pt.get("cube.valid_threshold", 0.3);
   m_transmitRate = pt.get("cube.transmit_rate", 0.5);
}

CubePlugin::CubePlugin(PluginManager *manager, CubeSettings cfg)
{
   this->BasePlugin::Construct(manager, cfg.m_id, PRIORITY_MED);
   m_cfg = cfg;
   m_validTime = 0;
}

bool CubePlugin::Init(PluginManager *manager, const std::string &cfgFile)
{
   osg::ref_ptr<osgART::GenericTracker> tracker = m_manager->ConstructTracker(m_cfg.m_arCfgPath);
   if (tracker == NULL || tracker->getMarkerCount() != 1 + 1) // HACK, marker file needs at least two markers defined for some reason
   {
      cout << "Cube should track one multi marker. Got: " << tracker->getMarkerCount() << endl;
      return false;
   }

   m_marker = tracker->getMarker(0);
   m_marker->setActive(true);
   m_marker->setRotationalSmoothing(m_cfg.m_rotateSmoothing);
   m_marker->setTranslationalSmoothing(m_cfg.m_translateSmoothing);

   m_camTracker = new CamTracker(m_manager->GetProjection(), m_marker);
   m_cubeTrackable = new TrackableOffsetCube(m_camTracker, m_cfg.m_offset);

   osg::ref_ptr<osg::MatrixTransform> transMat = new osgART::ARTTransform(m_marker);

   m_sceneGroup = new osg::Group();

   DrawHeadingTriangle(m_sceneGroup, m_cfg.m_cubeSize / 2 + 20.0f, 10.0f, m_cfg.m_cubeSize);

   transMat->addChild(m_sceneGroup);
   m_manager->AddScene(transMat.get());

   return true;
}

void CubePlugin::AnnounceClients(std::vector<Plugin*> &clients)
{
   m_tableRef = TablePlugin::getTableRef(clients);

   if (m_tableRef == NULL)
   {
      cout << "A table reference wasn't found. This is needed for tracking." << endl;
      throw exception();
   }
}

void CubePlugin::IncomingFrame(osgART::GenericVideo* sourceVid, osg::Timer_t now, double elapsed)
{
   m_elapsedTime += elapsed;

   if (m_marker->isValid() && m_tableRef->CanHasTracking()->hasVision()) m_validTime += elapsed;
   else m_validTime = 0;

   if (m_elapsedTime < m_cfg.m_transmitRate || m_validTime < m_cfg.m_validThreshold) return;

   m_elapsedTime = fmod(m_elapsedTime, m_cfg.m_transmitRate);

   double orientation = m_cubeTrackable->GetHeading(m_tableRef->Surface());
   osg::Vec2 position = m_cubeTrackable->SurfaceLocation(m_tableRef->Surface());

   cube_t message;
   message.info = m_manager->GetInfo(now);
   message.id = (char*) Id().c_str();
   message.orientation = orientation;
   message.position[0] = position.x();
   message.position[1] = position.y();

   lcm_t *comms = m_manager->GetComms();
   cube_t_publish(comms, Cube::CHANNEL.c_str(), &message); // NOTE: a result of 0 is "success"
}

void CubePlugin::IncludeInScene(osg::Node* child)
{
   m_sceneGroup->addChild(child);
}

CamTracker* CubePlugin::CanHasTracking()
{
   return m_camTracker;
}

std::vector<boost::shared_ptr<CubePlugin> > CubePlugin::LoadPlugins(PluginManager *manager, const std::string &cfgFile)
{
   CubeSettings baseCfg;

   baseCfg.load(cfgFile);

   string idxPath = baseCfg.m_CubesDir + string("/index");

   ifstream idxFile(idxPath.c_str(), ios::in);

   std::vector<boost::shared_ptr<CubePlugin> > cubes;

   std::string line;
   while (getline(idxFile, line))
   {
      std::string id;
      double size, x, y, z;
      std::stringstream lineStream;
      lineStream.str(line);

      lineStream >> id;
      lineStream >> size >> x >> y >> z;

      CubeSettings cubeCfg = baseCfg;
      cubeCfg.m_id = id;
      cubeCfg.m_cubeSize = size;
      cubeCfg.m_offset = osg::Vec3d(x, y, z);
      cubeCfg.m_arCfgPath = baseCfg.m_CubesDir + string("/") + id + string(".cfg");

      boost::shared_ptr<CubePlugin> cube (new CubePlugin(manager, cubeCfg));
      cubes.push_back(cube);
   }

   return cubes;
}






TrackableOffsetCube::TrackableOffsetCube(CamTracker *tracker, osg::Vec3d &offset)
   :m_tracker(tracker),
    m_offset(offset)
{}

double TrackableOffsetCube::GetHeading(TrackableSurface* surface)
{
   double rotation = m_tracker->GetHeading(surface->GetTracker());
   rotation = (rotation / M_PI) * 180.;

   return rotation;
}

osg::Vec2d TrackableOffsetCube::ScreenLocationCube()
{
   return m_tracker->ProjectPoint(osg::Vec4d(0, 0, 0, 1));
}

osg::Vec2d TrackableOffsetCube::ScreenLocationBase()
{
   osg::Vec4d fullOffset = osg::Vec4d(m_offset * -1, 1);
   return m_tracker->ProjectPoint(fullOffset);
}

osg::Vec2d TrackableOffsetCube::SurfaceLocation(TrackableSurface *surface)
{
   osg::Vec2d surfacePoint = m_tracker->GetPosition(surface->GetTracker());
   //surfacePoint += osg::Vec2d(m_offset.x(), m_offset.y());
   return surfacePoint;
}
