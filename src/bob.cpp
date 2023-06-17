#include "bob.hpp"

#include "config.hpp"

#include <cstddef>
#include <fstream>
#include <iostream>

#include <sys/inotify.h>
#include <unistd.h>

Bob::Bob(const ConfigManager &config_manager)
    : config_manager(config_manager)
{
    monitor_dir = config_manager.get_monitor_dir();
    output_dir = config_manager.get_output_dir();
} // Bob::Bob

void Bob::run()
{
    int inotify_instance = inotify_init();

    int watch_descriptor;

    if ((watch_descriptor = inotify_add_watch(inotify_instance, monitor_dir.c_str(), IN_ALL_EVENTS |IN_ONLYDIR)) == -1)
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

        std::cout << "Event captured\n";
        
        std::filesystem::path event_path(event.name);

        // Render the file
        std::ifstream input_file;
        input_file.open(monitor_dir / event_path);

        std::ofstream output_file;
        output_file.open(output_dir / event_path);

        output_file << input_file.rdbuf();

        input_file.close();
        output_file.close();
    }

    close(inotify_instance);
} // Bob::run