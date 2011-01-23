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
    double FindZRotation();
    bool hasVision();
protected:
    osg::Projection* m_proj;
    osgART::Marker *m_marker;

};
