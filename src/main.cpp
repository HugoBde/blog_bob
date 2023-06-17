#include "bob.hpp"
#include "config.hpp"

#include <fstream>
#include <iostream>

#include <boost/program_options.hpp>
#include <cmark-gfm.h>

int main(int argc, char *argv[])
{
    ConfigManager config_manager;
    config_manager.init(argc, argv);

    Bob builder(config_manager);

    builder.run();

    return 0;
}