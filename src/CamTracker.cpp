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

#include "CamTracker.h"
#include "util.h"
CamTracker::CamTracker(osg::Projection *proj, osgART::Marker *marker)
    :m_proj(proj),
      m_marker(marker)
{}

bool CamTracker::hasVision()
{
    return m_marker->isValid();
}

osg::Vec4d CamTracker::UnprojectToPlane(osg::Vec2d pt, osg::Matrixd proj, osg::Matrixd cam, osg::Vec4d V0, osg::Vec4d n)
{
    osg::Matrixd projInv = osg::Matrixd::inverse(proj);
    osg::Matrixd camInv = osg::Matrixd::inverse(cam);

    osg::Vec4d nearUn = osg::Vec4d(pt.x(), pt.y(), 0., 1.);
    osg::Vec4d farUn = osg::Vec4d(pt.x(), pt.y(), 1., 1.);
    osg::Vec4d nearPos = (nearUn * projInv) * camInv;
    osg::Vec4d farPos = (farUn * projInv) * camInv;

    nearPos /= nearPos.w();
    farPos /= farPos.w();

    osg::Vec4d ray = farPos - nearPos;

    osg::Vec4d realWorldPos = calcIntersect(nearPos, ray, V0, n);
    return realWorldPos;
}

osg::Vec4d CamTracker::UnprojectToPlane(osg::Vec2d pt, osg::Vec4d V0, osg::Vec4d n)
{
    return CamTracker::UnprojectToPlane(pt, m_proj->getMatrix(), m_marker->getTransform(), V0, n);
}

osg::Vec2d CamTracker::ProjectPoint(osg::Vec4d worldLoc, osg::Matrixd proj, osg::Matrixd cam)
{
    osg::Vec4d projected = (worldLoc * cam) * proj;

    projected /= projected.w();
    return osg::Vec2d(projected.x(), projected.y());
}

osg::Vec2d CamTracker::ProjectPoint(osg::Vec4d worldLoc)
{
    return CamTracker::ProjectPoint(worldLoc, m_proj->getMatrix(), m_marker->getTransform());
}

osg::Vec2d CamTracker::GetPosition(const CamTracker *reference)
{
    osg::Matrix myT = m_marker->getTransform();
    osg::Matrix oT = osg::Matrix::inverse(reference->m_marker->getTransform());
    osg::Vec4d P0 = osg::Vec4d(0, 0, 0, 1);

    P0 = P0 * (myT * oT);

    return osg::Vec2d(P0.x(), P0.y());
}

float CamTracker::GetHeading(const CamTracker *reference)
{
    osg::Matrix myT = m_marker->getTransform();
    osg::Matrix oT = osg::Matrix::inverse(reference->m_marker->getTransform());

    osg::Vec4d P0 = osg::Vec4d(0, 0, 0, 1);
    P0 = P0 * (myT * oT);

    osg::Vec4d P1 = osg::Vec4d(0, 10000, 0, 1);
    P1 = P1 * (myT * oT);

    float o = P1.y() - P0.y();
    float a = P1.x() - P0.x();

    float heading = atan2(o, a);
    return heading;
}
