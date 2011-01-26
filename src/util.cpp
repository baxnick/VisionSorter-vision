#include "util.h"

TablePlugin* getTableRef(std::vector<Plugin*> &plugins)
{
    TablePlugin *tableRef = NULL;

    std::vector<Plugin*>::iterator plugIt;
    for(plugIt = plugins.begin(); plugIt != plugins.end(); plugIt++)
    {
        TablePlugin* table = dynamic_cast<TablePlugin*>(*plugIt);

        if (table != NULL)
        {
            tableRef = table;
            break;
        }
    }

    return tableRef;
}

/*

      An example to rotate a model using a sequence of quaternion - Euler -
quaternion conversions.

      The Euler-to-conversion code is based on the equations on
www.euclideanspace.com. JG



      P.S: getQuatFromEuler is not neccessary. OSG's Matrix class has a public
method 'makeRotate(angle,axis)' which

      does the conversion



*/


void getEulerFromQuat(osg::Quat q, double& heading, double& attitude, double&
bank)

{
      double limit = 0.499999;




double sqx = q.x()*q.x(); double sqy = q.y()*q.y();

      double sqz = q.z()*q.z();



      double t = q.x()*q.y() + q.z()*q.w();

      if (t>limit) // gimbal lock ?

      {

            heading = 2 * atan2(q.x(),q.w());

            attitude = osg::PI_2;

            bank = 0;

      }

      else if (t<-limit)

      {

            heading = -2 * atan2(q.x(),q.w());

            attitude = - osg::PI_2;

            bank = 0;

      }

      else

      {



            heading = atan2(2*q.y()*q.w()-2*q.x()*q.z() , 1 - 2*sqy - 2*sqz);

            attitude = asin(2*t);

            bank = atan2(2*q.x()*q.w()-2*q.y()*q.z() , 1 - 2*sqx - 2*sqz);

      }

}

osg::Vec4 calcIntersect (osg::Vec4d P0, osg::Vec4d u, osg::Vec4d V0, osg::Vec4d n)
{
    u.normalize();
    n.normalize();

    osg::Vec4 w = P0 - V0;

    double s =( (-n) * w) / (n * u);

    if (s < 0) return osg::Vec4d(0,0,0,0);
    else return P0 + (u * s);
}
