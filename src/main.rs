use blog_bob::bob::Bob;

fn main() {
    let mut bob = Bob::new("./shit", "./out");

    bob.run();
}
