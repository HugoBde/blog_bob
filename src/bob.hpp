#pragma once

#include "config.hpp"

#include <filesystem>
#include <pqxx/connection>
#include <sstream>
#include <sys/inotify.h>

typedef struct inotify_event inotify_event;

class Bob // the Builder
{
  private:
    const ConfigManager &config_manager;
    pqxx::connection     db_conn;

    std::filesystem::path monitor_dir;
    std::filesystem::path output_dir;
    std::string           header;
    std::string           footer;

    void article_update(inotify_event * event);
    void article_delete(inotify_event * event);

    static std::string get_md_article_title(std::ifstream &md_article);

  public:
    Bob(const ConfigManager &config_manager);

    void run();

    // Private types
  private:
    enum BobException
    {
        ARTICLE_MISSING_TITLE
    };
};