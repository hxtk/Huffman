# Copyright: Peter Sanders. All rights reserved.
# Date: 2016-11-03

OBJ = obj
SRC = src
BUILD = build
TEST = test

CPP := g++
CPPFLAGS := -g -std=c++11 -I$(SRC) -pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wswitch-default -Wundef -Werror -Wno-unused

### General rules
all: $(BUILD)/huffman

clean:
	rm $(OBJ)/*.o $(BUILD)/* 

$(OBJ)/%.o: %(SRC)/%.h

$(BUILD)/huffman: $(OBJ)/main.o $(OBJ)/huffman.o $(SRC)/node.h
	$(CPP) $(CFLAGS) -o $@ $^

$(OBJ)/main.o: $(SRC)/main.cc
	$(CPP) $(CFLAGS) -o $@ -c $^

$(OBJ)/huffman.o: $(SRC)/huffman.h $(SRC)/node.h
	$(CPP) $(CFLAGS) -o $@ -c $(SRC)/huffman.cc

#$(OBJ)/node.o: $(SRC)/node.h
#	$(CPP) $(CFLAGS) -o $@ $^

.PHONY: clean all test

