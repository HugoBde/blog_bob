CC = clang++
BIN = build/blog_bob

$(BIN): src/main.cpp
	$(CC) -o $@ $^