BIN_DIR = bin
TARGETNAME=TextCompress

SRC_DIR = src
OBJ_DIR = obj
INC_DIR = include

TARGET = $(BIN_DIR)/$(TARGETNAME)
SRC = $(wildcard $(SRC_DIR)/*.cpp)
HDRS = $(wildcard $(INC_DIR)/*.h)
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

CC = g++
CPPFLAGS = -Iinclude -fopenmp
CFLAGS = -O2 -std=c++20 
LDFLAGS =
LDLIBS =

.PHONY: all clean debug
all: $(TARGET)

$(TARGET): $(OBJ) | $(BIN_DIR)
	$(CC) -g -fopenmp $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HDRS)  | $(OBJ_DIR)
	$(CC) -g $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

debug: $(OBJ) | $(BIN_DIR)
	$(CC) -g $(LDFLAGS) $^ $(LDLIBS) -o $(TARGET)_gdb
	gdb $(TARGET)

clean:
	rm -rv $(OBJ_DIR)
	rm $(TARGET)
	@if [ "$(BIN_DIR)" != "." ]; then\
		rm -rv $(BIN_DIR);\
	fi

test:
	echo "Running Tests"
	./runTests $(TARGET) tests/
	./runTests $(TARGET) customtests/