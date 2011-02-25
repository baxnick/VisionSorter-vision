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

#include <boost/shared_ptr.hpp>

#include "util.h"

#include "visionsystem.h"
#include <plugin/predef/TablePlugin.h>
#include <plugin/predef/CubePlugin.h>
#include <plugin/predef/BallPlugin.h>

using namespace std;

int main(int argc, char** argv)
{
   // Load vision system
   VisionSystem vs = VisionSystem(argc, argv);

   if (!vs.Init())
   {
      cout << "Failed to initialise plugin manager" << endl;
      exit(1);
   }

   // Load "plugins"
   TablePlugin *table = new TablePlugin(&vs);
   vs.LoadPlugin(table);

   BallPlugin *ball = new BallPlugin(&vs);
   vs.LoadPlugin(ball);

   std::vector < boost::shared_ptr<CubePlugin> > cubes = CubePlugin::LoadPlugins(&vs, vs.CfgPath());
   std::vector < boost::shared_ptr<CubePlugin> >::iterator iter;

   for (iter = cubes.begin(); iter != cubes.end(); iter++)
   {
      boost::shared_ptr<CubePlugin> cube = *iter;
      vs.LoadPlugin(cube.get());
   }


   // Second loading stage for plugins
   if (!vs.PrepareForRun())
   {
      cout << "Failed to initialise a plugin.." << endl;
      exit(1);
   }

   // Begin!
   vs.Run();
}
