#pragma once

#include "CamTracker.h"

#include <osg/Vec4>
#include <osg/Vec2>

class TrackableSurface
{
public:
    TrackableSurface(CamTracker *tracker, osg::Vec4d V0, osg::Vec4d n);
    osg::Vec2d Unproject(osg::Vec2d pt);
    double GetHeading();
protected:
    CamTracker *m_tracker;
    osg::Vec4d m_V0;
    osg::Vec4d m_n;

};


class TrackableOffsetCube
{
public:
    // NOTE: offset is cube position in relation to "ground" position
    TrackableOffsetCube(CamTracker *tracker, osg::Vec3d offset);
    osg::Vec2d ScreenLocationCube();
    osg::Vec2d ScreenLocationBase();
    osg::Vec2d SurfaceLocation(TrackableSurface* surface);
    double GetHeading(TrackableSurface* surface);
private:
    CamTracker *m_tracker;
    osg::Vec3d m_offset;

};
