#include "config.hpp"

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include <boost/program_options/option.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

Config::Config(int argc, char * argv[])
{
    // clang-format off
    prog_opts::options_description general_options("general");
    general_options.add_options()
        ("general.monitored_dir",    prog_opts::value<std::string>()->default_value("."), "Specify the directory to monitor for changes. Default is the current working directory")
        ("general.output_dir",       prog_opts::value<std::string>()->default_value("."), "Specify the directory to output to. Default is the current working directory")
        ("general.log_file",         prog_opts::value<std::string>()->default_value(""),  "Specify the directory to save logs in. Default is stderr")
        ("general.header_file",      prog_opts::value<std::string>()->default_value(""),  "Specify a file containing content to prepend to output files. Default is none")
        ("general.footer_file",      prog_opts::value<std::string>()->default_value(""),  "Specify a file containing content to append to output files. Default is none");

    prog_opts::options_description db_options("database");
    db_options.add_options()
        ("database.host",     prog_opts::value<std::string>()->default_value("127.0.0.1"),     "Specify the database server host")
        ("database.port",     prog_opts::value<uint16_t>()->default_value(5432),               "Specify the database server port")
        ("database.database", prog_opts::value<std::string>()->required(),                     "Specify the name of database to connect to")
        ("database.table",    prog_opts::value<std::string>()->required(),                     "Specify the name of the table to work on")
        ("database.user",     prog_opts::value<std::string>()->required(),                     "Specify the username to use for authentication")
        ("database.password", prog_opts::value<std::string>()->required(),                     "Specify the password to use for authentication");

    prog_opts::options_description config_file_options;
    config_file_options.add(general_options);
    config_file_options.add(db_options);

    prog_opts::options_description cmd_line_options("command_line");
    cmd_line_options.add_options()
        ("config,c", prog_opts::value<std::string>()->default_value("config.ini"), "Specify the file storing configuration");
    // clang-format on

    // Parse command line options
    prog_opts::store(prog_opts::parse_command_line(argc, argv, cmd_line_options), variables_map);

    std::string config_file_name = variables_map["config"].as<std::string>();

    // Parse config file
    try
    {
        prog_opts::store(
            prog_opts::parse_config_file(variables_map["config"].as<std::string>().c_str(), config_file_options),
            variables_map);
    }
    catch (std::exception & e)
    {
        std::string err_msg;
        err_msg.append(e.what());
        err_msg.append("\nSpecify configuration file using -c, --config\n");
        throw std::runtime_error(err_msg);
    }

    // Ensure all required parameters have been specified
    try
    {
        prog_opts::notify(variables_map);
    }
    catch (std::exception & e)
    {
        std::string err_msg;
        err_msg.append(e.what());
        err_msg.append("\nSee sample_config.ini for more details\n");
        throw std::runtime_error(err_msg);
    }
}

// void Config::init(int argc, char *argv[])
// {

// // Get config file option

// // Monitor Directory
// config.monitor_dir = variables_map["monitor"].as<std::string>();

// // Output Directory
// config.output_dir = variables_map["output"].as<std::string>();

// // Header and Footer file
// std::ifstream file;

// // Header
// std::stringstream header_file_content;
// file.open(variables_map["header"].as<std::string>());
// if (!file.is_open())
// {
//     // throw an excpetion header
// }
// header_file_content << file.rdbuf();
// config.header = header_file_content.str();
// file.close();

// // Footer
// std::stringstream footer_file_content;
// file.open(variables_map["footer"].as<std::string>());
// if (!file.is_open())
// {
//     // throw an excpetion footer
// }
// footer_file_content << file.rdbuf();
// config.footer = footer_file_content.str();
// file.close();
// }

template <typename T> const T & Config::get(const std::string & key) const
{
    return variables_map[key].as<T>();
}

template const std::string & Config::get<std::string>(const std::string & key) const;
template const uint16_t &    Config::get<uint16_t>(const std::string & key) const;