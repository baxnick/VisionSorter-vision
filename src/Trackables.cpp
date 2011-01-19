#include "Trackables.h"

TrackableSurface::TrackableSurface(CamTracker *tracker, osg::Vec4d V0, osg::Vec4d n)
    :m_tracker(tracker),
      m_V0(V0),
      m_n(n)
{}

double TrackableSurface::GetHeading()
{
    return m_tracker->FindZRotation();
}

osg::Vec2d TrackableSurface::Unproject(osg::Vec2d pt)
{
    osg::Vec4d result = m_tracker->UnprojectToPlane(pt, m_V0, m_n);
    return osg::Vec2d(result.x(), result.y());
}


TrackableOffsetCube::TrackableOffsetCube(CamTracker *tracker, osg::Vec3d offset)
    :m_tracker(tracker),
      m_offset(offset)
{}

double TrackableOffsetCube::GetHeading(TrackableSurface* surface)
{
    double rotation = m_tracker->FindZRotation() - surface->GetHeading() + 90;

    while (rotation >= 360) rotation -= 360;
    while (rotation < 0) rotation += 360;
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
    return surface->Unproject(this->ScreenLocationBase());
}
