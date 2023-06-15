#include <fstream>
#include <iostream>

#include <cmark-gfm.h>
#include <boost/program_options.hpp>

namespace prog_opts = boost::program_options;

namespace blog_bob {
    typedef struct {
        std::ifstream monitor;
        std::ofstream output;
        // std::ostream  log;
        std::string   header;
        std::string   footer;
    } config;
}

static blog_bob::config config;

void print_usage(const prog_opts::options_description & opts_desc)
{
    std::cerr << "Usage: blog_bob [OPTIONS]\n";
    std::cerr << opts_desc << '\n';
}

void monitor_opt_notifier(const std::string monitor_opt_value)
{
    std::cout << "monitor notifier: " << monitor_opt_value << '\n';

    config.monitor.open(monitor_opt_value);
}

void output_opt_notifer(const std::string output_opt_value)
{
    std::cout << "output notifier: " << output_opt_value << '\n';

    config.output.open(output_opt_value);
}

void log_opt_notifer(const std::string log_opt_value)
{
    std::cout << "log notifier: " << log_opt_value << '\n';
}

void header_opt_notifer(const std::string header_opt_value)
{
    std::cout << "header notifier: " << header_opt_value << '\n';
}

void footer_opt_notifer(const std::string footer_opt_value)
{
    std::cout << "footer notifier: " << footer_opt_value << '\n';
}

bool check_config(const prog_opts::variables_map & var_map)
{
    bool ret = true;

    if (!config.monitor.is_open())
    {
        std::cerr << "Failed to open directory '" << var_map["monitor"].as<std::string>() << "' for monitoring\n";
        ret = false;
    }

    if (!config.output.is_open())
    {
        std::cerr << "Failed to open directory '" << var_map["output"].as<std::string>() << "' for output\n";
        ret = false;
    }

    return ret;
}

int main(int argc, char *argv[])
{
    prog_opts::options_description interal_options("Internal Options");
    prog_opts::options_description display_options("Options");

    interal_options.add_options()
        ("help,h",    "Print this help message then exit")
        ("monitor,m", prog_opts::value<std::string>()->default_value(".")->notifier(monitor_opt_notifier), "Specify which folder to monitor for changes. Default is the current working directory")
        ("output,o",  prog_opts::value<std::string>()->default_value(".")->notifier(output_opt_notifer),   "Specify which folder to output to. Default is the current working directory")
        ("log,l",     prog_opts::value<std::string>()->default_value("")->notifier(log_opt_notifer),      "Specify which folder to save logs in. Default is stderr")
        ("header",    prog_opts::value<std::string>()->default_value("")->notifier(header_opt_notifer),   "Specify a file containing content to prepend to output files. Default is none")
        ("footer",    prog_opts::value<std::string>()->default_value("")->notifier(footer_opt_notifer),   "Specify a file containing content to append to output files. Default is none");

    display_options.add_options()
        ("help,h",    "Print this help message then exit")
        ("monitor,m", prog_opts::value<std::string>(), "Specify which folder to monitor for changes. Default is the current working directory")
        ("output,o",  prog_opts::value<std::string>(), "Specify which folder to output to. Default is the current working directory")
        ("log,l",     prog_opts::value<std::string>(), "Specify which folder to save logs in. Default is stderr")
        ("header",    prog_opts::value<std::string>(), "Specify a file containing content to prepend to output files. Default is none")
        ("footer",    prog_opts::value<std::string>(), "Specify a file containing content to append to output files. Default is none");


    prog_opts::variables_map var_map;

    prog_opts::store(prog_opts::parse_command_line(argc, argv, interal_options), var_map);

    if (var_map.count("help"))
    {
        print_usage(display_options);
        return 0;
    }

    prog_opts::notify(var_map);

    if (!check_config(var_map))
        return 1;

    return 0;
}