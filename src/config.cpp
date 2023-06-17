#include "config.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

// Initialise static config member
ConfigManager::Config ConfigManager::config;

ConfigManager::ConfigManager()
    : internal_options("Internal Options"),
      display_options("Options")
{
    internal_options.add_options()("help,h", "Print this help message then exit")(
        "monitor,m",
        prog_opts::value<std::string>()->default_value("."),
        "Specify which folder to monitor for changes. Default is the current working directory")(
        "output,o",
        prog_opts::value<std::string>()->default_value("."),
        "Specify which folder to output to. Default is the current working directory")(
        "log,l",
        prog_opts::value<std::string>()->default_value(""),
        "Specify which folder to save logs in. Default is stderr")(
        "header",
        prog_opts::value<std::string>()->default_value(""),
        "Specify a file containing content to prepend to output files. Default is none")(
        "footer",
        prog_opts::value<std::string>()->default_value(""),
        "Specify a file containing content to append to output files. Default is none");

    display_options.add_options()("help,h", "Print this help message then exit")(
        "monitor,m",
        prog_opts::value<std::string>(),
        "Specify which folder to monitor for changes. Default is the current working directory")(
        "output,o",
        prog_opts::value<std::string>(),
        "Specify which folder to output to. Default is the current working directory")(
        "log,l", prog_opts::value<std::string>(), "Specify which folder to save logs in. Default is stderr")(
        "header",
        prog_opts::value<std::string>(),
        "Specify a file containing content to prepend to output files. Default is none")(
        "footer",
        prog_opts::value<std::string>(),
        "Specify a file containing content to append to output files. Default is none");
}

void ConfigManager::init(int argc, char *argv[])
{
    prog_opts::store(prog_opts::parse_command_line(argc, argv, internal_options), variables_map);

    if (variables_map.count("help"))
    {
        print_usage();
        exit(0);
    }

    // Store config in internal struct

    // Monitor Directory
    config.monitor_dir = variables_map["monitor"].as<std::string>();

    // Output Directory
    config.output_dir = variables_map["output"].as<std::string>();

    // Header and Footer file
    std::ifstream file;

    // Header
    std::stringstream header_file_content;
    file.open(variables_map["header"].as<std::string>());
    if (!file.is_open())
    {
        // throw an excpetion header
    }
    header_file_content << file.rdbuf();
    config.header = header_file_content.str();
    file.close();

    // Footer
    std::stringstream footer_file_content;
    file.open(variables_map["footer"].as<std::string>());
    if (!file.is_open())
    {
        // throw an excpetion footer
    }
    footer_file_content << file.rdbuf();
    config.footer = footer_file_content.str();
    file.close();
}

const std::string &ConfigManager::get_monitor_dir() const
{
    return config.monitor_dir;
}

const std::string &ConfigManager::get_output_dir() const
{
    return config.output_dir;
}

const std::string &ConfigManager::get_header() const
{
    return config.header;
}

const std::string &ConfigManager::get_footer() const
{
    return config.footer;
}

void ConfigManager::print_usage() const
{
    std::cerr << "Usage: blog_bob [OPTIONS]\n";
    std::cerr << display_options << '\n';
}