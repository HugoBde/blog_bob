CC = clang++
BIN = build/blog_bob
LIBS = -lboost_program_options
FLAGS = 

$(BIN): src/main.cpp
	$(CC) -o $@ $^ $(FLAGS) $(LIBS)

clean:
	rm $(BIN)