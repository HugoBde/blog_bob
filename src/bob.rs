use std::error::Error;
use std::ffi::OsStr;
use std::path::PathBuf;

use configparser::ini::Ini;
use inotify::{Event, EventMask, Events, Inotify, WatchMask};
use postgres::{Client, Config, NoTls};
use pulldown_cmark::html::push_html;
use pulldown_cmark::Options as MarkdownOptions;
use pulldown_cmark::Parser;
use serde::Deserialize;
use time::Date;

pub struct Bob {
    inotify: Inotify,
    monitor_dir: PathBuf,
    output_dir: PathBuf,
    db_client: Client,
    md_options: MarkdownOptions,
}

impl Bob {
    pub fn new(config_path: &str) -> Result<Bob, Box<dyn Error>> {
        let mut config_parser = Ini::new();
        config_parser.load(config_path)?;

        let monitor_dir = config_parser
            .get("general", "monitor_dir")
            .ok_or("Missing \"monitor_dir\" parameter in \"general\" section")?;

        let output_dir = config_parser
            .get("general", "output_dir")
            .ok_or("Missing \"output_dir\" parameter in \"general\" section")?;

        let inotify = Inotify::init()?;

        inotify
            .watches()
            .add(&monitor_dir, WatchMask::ALL_EVENTS | WatchMask::ONLYDIR)?;

        let mut db_config = Config::new();

        db_config
            .host(
                config_parser
                    .get("database", "host")
                    .unwrap_or(String::from("localhost"))
                    .as_str(),
            )
            .port(config_parser.getuint("database", "port")?.unwrap_or(5432) as u16)
            .user(
                config_parser
                    .get("database", "user")
                    .ok_or("Missing \"user\" parameter in \"database\" section")?
                    .as_str(),
            )
            .password(
                config_parser
                    .get("database", "password")
                    .ok_or("Missing \"password\" parameter in \"database\" section")?
                    .as_str(),
            )
            .dbname(
                config_parser
                    .get("database", "dbname")
                    .ok_or("Missing \"dbname\" parameter in \"database\" section")?
                    .as_str(),
            );

        let mut md_options = MarkdownOptions::empty();

        md_options.insert(MarkdownOptions::ENABLE_STRIKETHROUGH);
        md_options.insert(MarkdownOptions::ENABLE_TABLES);

        Ok(Bob {
            inotify,
            monitor_dir: PathBuf::from(monitor_dir),
            output_dir: PathBuf::from(output_dir),
            db_client: db_config.connect(NoTls)?,
            md_options: md_options,
        })
    }

    pub fn run(&mut self) {
        let mut buffer = [0; 1024];

        println!(
            "Bob monitoring {} and writing to {}...",
            self.monitor_dir.display(),
            self.output_dir.display()
        );

        loop {
            match self.inotify.read_events_blocking(&mut buffer) {
                Ok(events) => {
                    if let Err(error) = self.process_events(events) {
                        eprintln!("{}", error);
                    }
                }
                Err(error) => eprintln!("{}", error),
            }
        }
    }

    fn process_events(&mut self, events: Events<'_>) -> Result<(), Box<dyn Error>> {
        for event in events {
            match event.mask {
                EventMask::MODIFY => self.update_article(event)?,
                EventMask::DELETE => self.delete_article(event)?,
                _ => {}
            }
        }

        Ok(())
    }

    fn update_article(&mut self, event: Event<&OsStr>) -> Result<(), Box<dyn Error>> {
        let mut input_file_name = PathBuf::from(&self.monitor_dir);

        input_file_name.push(event.name.unwrap());

        let file_content = std::fs::read_to_string(&input_file_name).unwrap();

        let mut parts = file_content.splitn(3, "---");

        let frontmatter_content = parts.nth(1).ok_or(format!(
            "Missing frontmatter in {}",
            input_file_name.display()
        ))?;

        let md_content = parts
            .nth(0) // Calling nth consumes all items up to and including the one returned. This
            // means that to get the next part we just call nth(0)
            .ok_or(format!("Missing markdown in {}", input_file_name.display()))?;

        // parse yaml
        let article_metadata: ArticleMetadata = serde_yaml::from_str(frontmatter_content)?;

        // parse md
        let md_parser = Parser::new_ext(md_content, self.md_options);

        let mut html_content = String::new();

        push_html(&mut html_content, md_parser);

        let mut output_file_name = PathBuf::from(&self.output_dir);
        output_file_name.push(event.name.unwrap());
        output_file_name.set_extension("html");
        std::fs::write(&output_file_name, html_content).unwrap();

        self.db_client.execute(
            "INSERT INTO blog_articles VALUES(DEFAULT, $1, $2, $3 , CURRENT_DATE, $4);",
            &[
                &article_metadata.title,
                &output_file_name.to_str(),
                &article_metadata.pub_date,
                &article_metadata.tags,
            ],
        )?;

        println!("Updated article \"{}\"", article_metadata.title);

        Ok(())
    }

    fn delete_article(&mut self, event: Event<&OsStr>) -> Result<(), Box<dyn Error>> {
        let mut file_name = PathBuf::from(event.name.unwrap());
        file_name.set_extension("");

        self.db_client.execute(
            "DELETE FROM blog_articles WHERE path ~ 'shit';",
            &[/* &file_name.to_str() */],
        )?;

        println!(
            "Deleted article \"{}\"",
            event.name.unwrap().to_string_lossy()
        );

        Ok(())
    }
}

#[allow(dead_code)]
#[derive(Debug, Deserialize)]
struct ArticleMetadata {
    title: String,
    #[serde(with = "date_format")]
    pub_date: Date,
    tags: Vec<String>,
}

time::serde::format_description!(date_format, Date, "[year]-[month]-[day]");
