use std::error::Error;
use std::ffi::OsStr;
use std::path::PathBuf;

use configparser::ini::Ini;
use inotify::{Event, EventMask, Events, Inotify, WatchMask};
use pulldown_cmark::html::push_html;
use pulldown_cmark::{Options, Parser};
use serde::Deserialize;
use time::Date;

pub struct Bob {
    inotify: Inotify,
    monitor_dir: PathBuf,
    output_dir: PathBuf,
}

impl Bob {
    pub fn new(config_path: &str) -> Result<Bob, Box<dyn Error>> {
        let mut config_parser = Ini::new();
        config_parser.load(config_path)?;

        let monitor_dir = config_parser
            .get("general", "monitor_dir")
            .ok_or("Missing monitor_dir parameter in \"general\" section")?;

        let output_dir = config_parser
            .get("general", "output_dir")
            .ok_or("Missing output_dir parameter in \"general\" section")?;

        let inotify = Inotify::init()?;

        inotify
            .watches()
            .add(&monitor_dir, WatchMask::ALL_EVENTS | WatchMask::ONLYDIR)?;

        Ok(Bob {
            inotify,
            monitor_dir: PathBuf::from(monitor_dir),
            output_dir: PathBuf::from(output_dir),
        })
    }

    pub fn run(&mut self) {
        let mut buffer = [0; 1024];

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

    fn process_events(&self, events: Events<'_>) -> Result<(), Box<dyn Error>> {
        for event in events {
            match event.mask {
                EventMask::MODIFY => self.update_article(event)?,
                EventMask::DELETE => self.delete_article(event)?,
                _ => {}
            }
        }

        Ok(())
    }

    fn update_article(&self, event: Event<&OsStr>) -> Result<(), Box<dyn Error>> {
        let mut file_name = PathBuf::from(&self.monitor_dir);

        file_name.push(event.name.unwrap());

        println!("{:?}", file_name);

        let file_content = std::fs::read_to_string(&file_name).unwrap();

        let mut parts = file_content.splitn(3, "---");

        let frontmatter_content = parts.nth(1).ok_or("Missing markdown in file")?;
        let md_content = parts.nth(2).ok_or("Missing markdown in file")?;

        // parse yaml
        let article_metadata: ArticleMetadata = serde_yaml::from_str(frontmatter_content)?;

        // parse md
        let mut md_options = Options::empty();

        md_options.insert(Options::ENABLE_STRIKETHROUGH);

        md_options.insert(Options::ENABLE_TABLES);

        let md_parser = Parser::new_ext(md_content, md_options);

        let mut html = String::new();

        push_html(&mut html, md_parser);

        std::fs::write("blog/shit.html", html).unwrap();

        // do SQL query

        Ok(())
    }

    fn delete_article(&self, event: Event<&OsStr>) -> Result<(), Box<dyn Error>> {
        Ok(())
    }
}

#[derive(Debug, Deserialize)]

struct ArticleMetadata {
    title: String,
    #[serde(with = "date_format")]
    pub_date: Date,
    tags: Vec<String>,
}

time::serde::format_description!(date_format, Date, "[year]-[month]-[day]");
