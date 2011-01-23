#include "Plugin.h"

BasePlugin::BasePlugin(){}

BasePlugin::BasePlugin(PluginManager *manager, std::string id, int priority)
{
    Construct(manager, id, priority);
}

void BasePlugin::Construct(PluginManager *manager, std::string id, int priority)
{
    m_manager = manager;
    m_id = id;
    m_priority = priority;
    m_numTrackedItems = 0;
    m_elapsedTime = 0;
    m_validTime = 0;
}

std::string BasePlugin::Id()
{
    return m_id;
}

int BasePlugin::Priority()
{
    return m_priority;
}

int BasePlugin::TrackedItemCount()
{
    return m_numTrackedItems;
}
