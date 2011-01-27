#include <boost/shared_ptr.hpp>

#include "util.h"

#include "visionsystem.h"
#include "Plugin.h"
#include "TablePlugin.h"
#include "CubePlugin.h"
#include "BallPlugin.h"

using namespace std;

int main(int argc, char** argv)
{
    // Load vision system
    VisionSystem vs = VisionSystem(argc, argv);

    if (!vs.Init())
    {
        cout << "Failed to initialise plugin manager" << endl;
        exit(1);
    }

    // Load "plugins"
    TablePlugin *table = new TablePlugin(&vs);
    vs.LoadPlugin(table);

    BallPlugin *ball = new BallPlugin(&vs);
    vs.LoadPlugin(ball);

    std::vector < boost::shared_ptr<CubePlugin> > cubes = CubePlugin::LoadPlugins(&vs, vs.CfgPath());
    std::vector < boost::shared_ptr<CubePlugin> >::iterator iter;

    for (iter = cubes.begin(); iter != cubes.end(); iter++)
    {
        boost::shared_ptr<CubePlugin> cube = *iter;
        vs.LoadPlugin(cube.get());
    }


    // Second loading stage for plugins
    if (!vs.PrepareForRun())
    {
        cout << "Failed to initialise a plugin.." << endl;
        exit(1);
    }

    // Begin!
    vs.Run();
}
