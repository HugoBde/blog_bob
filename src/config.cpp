#include "config.hpp"

#include <cstdlib>
#include <iostream>

#include <boost/program_options/options_description.hpp>

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

    if (variables_map.count("help")) {
        std::cout << display_options << '\n';
        exit(0);
    }
}

void print_usage(const prog_opts::options_description &opts_desc)
{
    std::cerr << "Usage: blog_bob [OPTIONS]\n";
    std::cerr << opts_desc << '\n';
}