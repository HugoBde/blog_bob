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
    // General
    std::filesystem::path monitor_dir;
    std::filesystem::path output_dir;
    std::string           header;
    std::string           footer;

    // Database
    pqxx::connection db_conn;
    std::string      db_table;

    void article_update(inotify_event * event);
    void article_delete(inotify_event * event);

    static std::string get_md_article_title(std::ifstream & md_article);

  public:
    static Bob create(const Config & config);
    Bob(const Config & config, const std::string & db_conn_string);

    void run();
};