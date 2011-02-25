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
