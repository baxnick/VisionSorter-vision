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

#include <osgART/VideoManager>
#include <osg/Timer>
#include <osg/Node>
#include <osg/Vec2>

#include <string>
#include <vector>

#include "PluginManager.h"
#include "CamTracker.h"

#define TABLE_ID "TABLE"
#define BALL_ID "BALL"
#define CUBE_ID "CUBE"

#define PRIORITY_HIGH 0
#define PRIORITY_MED 10
#define PRIORITY_LOW 20

class Plugin
{
public:
   virtual std::string Id() = 0;
   virtual int Priority() = 0;
   virtual int TrackedItemCount() = 0;
   virtual bool Init(PluginManager *manager, const std::string &cfgFile) = 0;
   virtual void AnnounceClients(std::vector<Plugin*> &clients) = 0;

   virtual void IncomingFrame(osgART::GenericVideo* sourceVid, osg::Timer_t now, double elapsed) = 0;
   virtual void IncludeInScene(osg::Node* child) = 0;

   virtual CamTracker* CanHasTracking() = 0;

   virtual ~Plugin() {};
};

class BasePlugin : public Plugin
{
public:
   virtual std::string Id();
   virtual int Priority();
   virtual int TrackedItemCount();

   BasePlugin(PluginManager *manager, std::string id, int priority);
protected:
   BasePlugin();
   void Construct(PluginManager *manager, std::string id, int priority);
   PluginManager *m_manager;
   int m_numTrackedItems;
   double m_elapsedTime;
   double m_validTime;
private:
   std::string m_id;
   int m_priority;
};
