#include "bob.hpp"

#include "config.hpp"

#include <cstddef>
#include <cstdint>
#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/inotify.h>

#include <cmark-gfm.h>
#include <pqxx/connection.hxx>
#include <pqxx/transaction.hxx>

Bob Bob::create(const Config & config)
{
    // Build database connection string
    std::string db_conn_string = "host=" + config.get<std::string>("database.host") +
                                 " port=" + std::to_string(config.get<std::uint16_t>("database.port")) +
                                 " dbname=" + config.get<std::string>("database.database") +
                                 " user=" + config.get<std::string>("database.user") +
                                 " password=" + config.get<std::string>("database.password");

    return Bob(config, db_conn_string);
}

Bob::Bob(const Config & config, const std::string & db_conn_string)
    : db_conn(db_conn_string)
{
    monitor_dir = config.get<std::string>("general.monitored_dir");
    output_dir  = config.get<std::string>("general.output_dir");

    db_table = config.get<std::string>("database.table");

    // Read header file
    std::string       header_file_name = config.get<std::string>("general.header_file");
    std::ifstream     header_file(header_file_name);
    std::stringstream header_ss;

    if (!header_file.is_open())
        throw std::runtime_error("Failed to open header file: " + header_file_name);

    header_ss << header_file.rdbuf();
    header = header_ss.str();

    // Read footer file
    std::string       footer_file_name = config.get<std::string>("general.footer_file");
    std::ifstream     footer_file(footer_file_name);
    std::stringstream footer_ss;

    if (!footer_file.is_open())
        throw std::runtime_error("Failed to open footer file: " + footer_file_name);

    footer_ss << footer_file.rdbuf();
    footer = footer_ss.str();
}

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

    while (true)
    {
        uint8_t buffer[2048];
        ssize_t read_count;

        if ((read_count = read(inotify_instance, &buffer, sizeof(buffer))) == -1)
        {
            std::cerr << "Error reading from Inotify. Errno: " << errno << '\n';
            return;
        }

        inotify_event * event;

        for (uint8_t * ptr = buffer; ptr - buffer < read_count; ptr += sizeof(inotify_event) + event->len)
        {
            event = (inotify_event *)ptr;

            switch (event->mask)
            {
                case IN_MODIFY:
                    article_update(event);
                    break;
                case IN_DELETE:
                    article_delete(event);
                    break;
            }
        }
    }

    close(inotify_instance);
}

void Bob::article_update(inotify_event * event)
{
    // Read modified file
    std::filesystem::path input_file_path(event->name);

    if (input_file_path.extension() != ".md")
        return;

    std::ifstream input_file(monitor_dir / input_file_path);

    std::string blog_title;

    try
    {
        blog_title = get_md_article_title(input_file);
    }
    catch (const std::exception & e)
    {
        std::cout << input_file_path << ": " << e.what() << std::endl;
        return;
    }

    std::stringstream md_content;
    md_content << input_file.rdbuf();

    // Convert Markdown to HTML
    char * html_content = cmark_markdown_to_html(md_content.str().c_str(), md_content.str().size(), CMARK_OPT_DEFAULT);

    // Create output file
    std::filesystem::path output_file_path(input_file_path);
    output_file_path.replace_extension("html");
    std::ofstream output_file(output_dir / output_file_path);

    // Write to output file
    output_file << header;
    output_file << html_content;
    output_file << footer;

    // Free heap allocated HTML content
    free(html_content);

    // Update database
    pqxx::work db_transaction(db_conn);

    pqxx::result res = db_transaction.exec("SELECT * FROM " + db_table + " WHERE title='" + blog_title + "';");

    // If row exist, update it, otherwise create a new entry
    if (res.size() == 1)
    {
        db_transaction.exec("UPDATE " + db_table + " SET update_date=CURRENT_DATE WHERE title='" + blog_title + "';");
        db_transaction.commit();
        std::cout << "Updated: \"" << blog_title << "\"\n";
    }
    else
    {
        db_transaction.exec("INSERT INTO " + db_table + " VALUES ('" + blog_title + "', CURRENT_DATE);");
        db_transaction.commit();
        std::cout << "Created: \"" << blog_title << "\" for \"" << event->name << "\"\n";
    }
}

void Bob::article_delete(inotify_event * event)
{
}

std::string Bob::get_md_article_title(std::ifstream & md_article)
{
    // Just in case
    md_article.seekg(0);

    std::string first_line;
    std::getline(md_article, first_line);

    // Reset reading cursor
    md_article.seekg(0);

    // Title should be of the form "# <title>" in Markdown
    if (first_line[0] == '#' && first_line[1] == ' ')
    {
        return first_line.substr(2);
    }

    throw std::runtime_error("missing title");
}
