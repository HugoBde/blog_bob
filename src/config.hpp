#pragma once

#include "boost/program_options.hpp"

#include <fstream>

#include <boost/program_options/options_description.hpp>

namespace prog_opts = boost::program_options;

class Config
{
  private:
    prog_opts::variables_map variables_map;

  public:
    Config(int argc, char * argv[]);

    template <typename T> const T & get(const std::string & key) const;

    void print_usage() const;
};
