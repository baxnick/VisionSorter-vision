#pragma once

#include <osg/Matrix>
#include <osg/Vec4>
#include <osg/Quat>


osg::Vec4 calcIntersect (osg::Vec4d P0, osg::Vec4d u, osg::Vec4d V0, osg::Vec4d n);
void getEulerFromQuat(osg::Quat q, double& heading, double& attitude, double& bank);

float getPitch(float w, float x, float y, float z);
float getYaw(float w, float x, float y, float z);
float getRoll(float w, float x, float y, float z);
