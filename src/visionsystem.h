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

#include <plugin/PluginManager.h>
#include <plugin/Plugin.h>

#include <osgProducer/Viewer>

namespace General
{
    const std::string INI_PATH("Data/vision.ini");
}

class VisionSystem : public PluginManager
{
public:
    VisionSystem(int argc, char** argv);
    virtual ~VisionSystem(){};
    bool Init();
    bool LoadPlugin(Plugin *plugin);
    bool PrepareForRun();
    void Run();

    std::string CfgPath();
    virtual bool ShowUi();
    void RegisterViewEvent (osgGA::GUIEventHandler *handler);
    void AddScene (osg::Node *node);
    osg::ref_ptr<osgART::GenericTracker> ConstructTracker(std::string trackerCfg);
    osg::Projection* GetProjection();

    // TODO import LCM
    lcm_t* GetComms();
    info_t GetInfo(osg::Timer_t stamp);

private:
    bool m_showUi;
    int m_argc;
    char** m_argv;
    std::string m_iniFile;
    std::string m_cameraDat;
    std::string m_id;
    std::vector<Plugin*> m_plugins;
    std::vector<osg::ref_ptr<osgART::GenericTracker> > m_trackers;

    osg::Timer_t m_startTime;
    osg::Projection* m_projectionMatrix;
    osgProducer::Viewer m_viewer;
    osg::Group* m_sceneGroup;
    osg::ref_ptr<osgART::GenericVideo> m_video;
    lcm_t* m_lcm;

    bool Done();
};
