#pragma once

#include "boost/program_options.hpp"

#include <fstream>

namespace prog_opts = boost::program_options;

class ConfigManager
{
  private:
    class Config
    {
        std::string monitor_dir;
        std::string output_dir;
    };

    static Config config;

    prog_opts::options_description internal_options;
    prog_opts::options_description display_options;
    prog_opts::variables_map       variables_map;

    static Config &get_config();

  public:
    ConfigManager();

    void init(int argc, char *argv[]);

    const std::string &get_monitor_dir();
    const std::string &get_output_dir();
};
