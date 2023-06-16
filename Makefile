CC = clang++
LIBS = -lboost_program_options
FLAGS = 

BUILD_DIR = build
OBJ_DIR   = $(BUILD_DIR)/obj

BIN = $(BUILD_DIR)/blog_bob

OBJS = $(OBJ_DIR)/main.o    \
       $(OBJ_DIR)/bob.o     \
	   $(OBJ_DIR)/config.o
	   

$(BIN): $(OBJS) | $(BUILD_DIR)
	$(CC) -o $@ $^ $(FLAGS) $(LIBS)

$(OBJ_DIR)/%.o: src/%.cpp | $(OBJ_DIR)
	$(CC) -o $@ -c $< $(FLAGS)

$(BUILD_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	rm $(BIN) -f
	rm $(OBJS) -f