#include "visionsystem.h"

VisionSystem::VisionSystem(std::string id, int argc, char **argv)
    :m_id(id),
      m_argc(argc),
      m_argv(argv)
{}


osgART::GenericTracker* VisionSystem::ConstructTracker(std::string dataFile){return NULL;}

void VisionSystem::AddScene(osg::Node node){}
void VisionSystem::RegisterViewEvent(osgGA::GUIEventHandler *handler){}

lcm_t* VisionSystem::GetComms(){return NULL;}
info_t VisionSystem::GetInfo(){return info_t();}

bool VisionSystem::Init(){return false;}
bool VisionSystem::LoadPlugin(Plugin *plugin){ return false;}
bool VisionSystem::PrepareForRun(){ return false;}

void VisionSystem::Run(){}
