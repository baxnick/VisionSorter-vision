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

#include <osg/Vec4>
#include <osg/Vec2>
#include <osg/Matrix>

#include <osgART/TrackerManager>

class CamTracker
{
public:
   CamTracker(osg::Projection *proj, osgART::Marker *marker);
   osg::Vec4d UnprojectToPlane(osg::Vec2d pt, osg::Vec4d V0, osg::Vec4d n);
   static osg::Vec4d UnprojectToPlane(osg::Vec2d pt, osg::Matrixd proj, osg::Matrixd cam, osg::Vec4d V0, osg::Vec4d n);
   osg::Vec2d ProjectPoint(osg::Vec4d worldLoc);
   static osg::Vec2d ProjectPoint(osg::Vec4d worldLoc, osg::Matrixd proj, osg::Matrixd cam);
   double FindHeading();
   double FindBanking();
   double FindAttitude();

   osg::Vec2d GetPosition(const CamTracker *reference);
   float GetHeading(const CamTracker *reference);

   bool hasVision();
protected:
   osg::Projection* m_proj;
   osgART::Marker *m_marker;

};
