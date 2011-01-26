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

double CamTracker::FindZRotation()
{
    double heading, attitude, bank;

    osg::Matrixd source = m_marker->getTransform();
    osg::Quat rotation = source.getRotate();

    getEulerFromQuat(rotation, heading, attitude, bank);
    double hDeg = heading * (180. / M_PI);

    return hDeg;
}

double CamTracker::FindBanking()
{
    double heading, attitude, bank;

    osg::Matrixd source = m_marker->getTransform();
    osg::Quat rotation = source.getRotate();

    getEulerFromQuat(rotation, heading, attitude, bank);
    double bDeg = bank * (180. / M_PI);

    return bDeg;
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
