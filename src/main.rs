use blog_bob::bob::Bob;
use std::error::Error;

fn main() -> Result<(), Box<dyn Error>> {
    let mut bob = Bob::new("./config.ini")?;

    bob.run();

    Ok(())
}
