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

#pragma once

#include <osg/Group>
#include <osg/Node>
#include <osg/Timer>
#include <osgGA/GUIEventHandler>
#include <osgART/TrackerManager>
#include <lcm/lcm.h>

#include "lcmtypes/info_t.h"

class PluginManager
{
public:
   virtual bool ShowUi() = 0;
   virtual void RegisterViewEvent (osgGA::GUIEventHandler *handler) = 0;
   virtual void AddScene (osg::Node *node) = 0;
   virtual osg::ref_ptr<osgART::GenericTracker> ConstructTracker(std::string trackerCfg) = 0;
   virtual osg::Projection* GetProjection() = 0;

   // TODO import LCM
   virtual lcm_t* GetComms() = 0;
   virtual info_t GetInfo(osg::Timer_t stamp) = 0;

   virtual ~PluginManager() {};
};
