#include "Plugin.h"

BasePlugin::BasePlugin(PluginManager *manager, std::string id, int priority)
    :m_manager(manager),
      m_id(id),
      m_priority(priority)
{}

std::string BasePlugin::Id()
{
    return m_id;
}

int BasePlugin::Priority()
{
    return m_priority;
}
