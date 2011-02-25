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

#include "util.h"

osg::Vec4 calcIntersect (osg::Vec4d P0, osg::Vec4d u, osg::Vec4d V0, osg::Vec4d n)
{
   u.normalize();
   n.normalize();

   osg::Vec4 w = P0 - V0;

   double s =( (-n) * w) / (n * u);

   if (s < 0) return osg::Vec4d(0,0,0,0);
   else return P0 + (u * s);
}
