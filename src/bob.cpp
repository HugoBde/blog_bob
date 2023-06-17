#include "bob.hpp"

#include "config.hpp"

#include <cstddef>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/inotify.h>

#include <cmark-gfm.h>

Bob::Bob(const ConfigManager &config_manager)
    : config_manager(config_manager)
{
    monitor_dir = config_manager.get_monitor_dir();
    output_dir  = config_manager.get_output_dir();
    header      = config_manager.get_header();
    footer      = config_manager.get_footer();
} // Bob::Bob

void Bob::run()
{
    int inotify_instance = inotify_init();

    int watch_descriptor;

    if ((watch_descriptor = inotify_add_watch(inotify_instance, monitor_dir.c_str(), IN_ALL_EVENTS | IN_ONLYDIR)) == -1)
    {
        std::cout << errno << '\n';
        close(inotify_instance);
        return;
    }

    std::cout << "Watching " << monitor_dir.c_str() << "...\n";

    struct inotify_event event;

    while (true)
    {
        if (read(inotify_instance, &event, sizeof(event) + NAME_MAX + 1) == -1)
        {
            std::cerr << "Error reading from Inotify. Errno: " << errno << '\n';
            return;
        }

        // Ignore all events other than IN_MODIFY
        if (event.mask != IN_MODIFY)
            continue;

        std::cout << "Updated: " << event.name << '\n';

        // Convert the modified file's name to a path
        std::filesystem::path input_file_path(event.name);

        // Read modified file
        std::ifstream input_file;
        input_file.open(monitor_dir / input_file_path);
        std::stringstream md_content;
        md_content << input_file.rdbuf();
        input_file.close();

        // Convert Markdown to HTML
        char *html_content =
            cmark_markdown_to_html(md_content.str().c_str(), md_content.str().size(), CMARK_OPT_DEFAULT);

        // Create output file
        std::filesystem::path output_file_path(input_file_path);
        output_file_path.replace_extension("html");
        std::ofstream output_file;
        output_file.open(output_dir / output_file_path);

        output_file << header;

        output_file << html_content;

        output_file << footer;

        // Close output file
        output_file.close();

        // Free heap allocated HTML content
        free(html_content);
    }

    close(inotify_instance);
} // Bob::run
