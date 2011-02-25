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
