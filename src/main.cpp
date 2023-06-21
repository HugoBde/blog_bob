#include "bob.hpp"
#include "config.hpp"

#include <exception>
#include <fstream>
#include <iostream>

#include <boost/program_options.hpp>
#include <cmark-gfm.h>

int main(int argc, char * argv[])
{
    try
    {
        Config config(argc, argv);
        Bob    bob = Bob::create(config);

        bob.run();
    }
    catch (const std::exception & e)
    {
        std::cout << e.what() << std::endl;
    }

    return 0;
}