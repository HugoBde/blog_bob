use std::ffi::OsStr;
use std::path::PathBuf;

use inotify::{Event, EventMask, Inotify, WatchMask};
use pulldown_cmark::html::push_html;
use pulldown_cmark::{Options, Parser};
use serde::Deserialize;
use time::Date;

pub struct Bob {
    inotify:     Inotify,
    monitor_dir: PathBuf,
    output_dir:  PathBuf,
}

impl Bob {
    pub fn new(monitor_dir: &str, output_dir: &str) -> Bob {

        let inotify = Inotify::init().unwrap();

        inotify.watches().add(monitor_dir, WatchMask::ALL_EVENTS | WatchMask::ONLYDIR).unwrap();

        Bob {
            inotify,
            monitor_dir: PathBuf::from(monitor_dir),
            output_dir: PathBuf::from(output_dir),
        }
    }

    pub fn run(&mut self) {

        let mut buffer = [0; 1024];

        loop {

            let events = self.inotify.read_events_blocking(&mut buffer).unwrap();

            for event in events {

                println!("{:?}", event);

                match event.mask {
                    EventMask::MODIFY => self.update_article(event),
                    _ => {}
                }
            }
        }
    }

    fn update_article(&self, event: Event<&OsStr>) {

        let mut file_name = PathBuf::from(&self.monitor_dir);

        file_name.push(event.name.unwrap());

        println!("{:?}", file_name);

        let file_content = std::fs::read_to_string(&file_name).unwrap();

        let parts: Vec<&str> = file_content.splitn(3, "---").collect();

        if parts.len() != 3 {

            eprintln!("frontmatter missing in {}", file_name.display());
        }

        // parse yaml
        let article_metadata: ArticleMetadata = serde_yaml::from_str(parts[1]).unwrap();

        println!("METADATA:\n{:#?}", article_metadata);

        // parse md
        let mut md_options = Options::empty();

        md_options.insert(Options::ENABLE_STRIKETHROUGH);

        md_options.insert(Options::ENABLE_TABLES);

        let md_parser = Parser::new_ext(parts[2], md_options);

        let mut html = String::new();

        push_html(&mut html, md_parser);

        println!("{}", html);
    }
}

#[derive(Debug, Deserialize)]

struct ArticleMetadata {
    title:    String,
    pub_date: Date,
    tags:     Vec<String>,
}
