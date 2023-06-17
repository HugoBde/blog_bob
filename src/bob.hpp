#pragma once

#include "config.hpp"

#include <filesystem>
#include <sstream>

class Bob // the Builder
{
  private:
    const ConfigManager &config_manager;

    std::filesystem::path monitor_dir;
    std::filesystem::path output_dir;
    std::string           header;
    std::string           footer;

  public:
    Bob(const ConfigManager &config_manager);

    void run();
};