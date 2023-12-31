![GitHub Workflow Status (with event)](https://img.shields.io/github/actions/workflow/status/HugoBde/blog_bob/build-test.yml)

---

# BLOG_BOB

## What is blog_bob
`blob_bob` is a microservice I developed to support my continuously deployed blog. It monitors a folder for new or updated markdown files, and converts them to HTML blog articles in sub millisecond time. It also updates a supporting PostgreSQL database which catalogues all the articles and helps the main backend server render the blog homepage.

## Why the name "blog_bob"?
I am a huge fan of Bob the Builder, and attribute my "can-do" attitude and love for tinkering and building things to him. Since `blog_bob` helps me **build** my blog, I wanted to dedicate this project to Bob the Builder as a thank you for the positive impact he has had on my life.

![Bob the Builder](https://upload.wikimedia.org/wikipedia/en/thumb/c/c5/Bob_the_builder.jpg/220px-Bob_the_builder.jpg)

## Todo
- [ ] update database if monitored folder content has changed on startup
- [ ] queue request to database if not available
- [ ] loggging to specified file

## Lessons learnt
Rust is so much better, I miss Result and Option
Update 10-07-2023: I rewrote this in Rust and it's so much better
