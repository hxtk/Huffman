# Copyright: Peter Sanders. All rights reserved.
# Date: 2016-11-03

OBJ = obj
SRC = src
BUILD = build
TEST = test

CPP := g++
CFLAGS := -g -std=c++11 -I$(SRC)/ -lgflags -lglog -Wall -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wswitch-default -Wundef -Wno-unused

### General rules
all: $(BUILD)/huffman $(TEST)/bitstring

clean:
	rm -r $(OBJ)/* $(BUILD)/* 
	true

test: $(TEST)/bitstring $(TEST)/huffman

$(OBJ)/%.o: %(SRC)/%.h

$(OBJ)/compression/huffman/%.o:
	mkdir $(OBJ)/huffman

$(OBJ)/base/%:
	mkdir $(OBJ)/base

$(BUILD)/huffman: $(OBJ)/main.o $(OBJ)/compression/huffman/huffman.o $(OBJ)/base/bitstring.o
	$(CPP) $(CFLAGS) -o $@ $^

$(OBJ)/main.o: $(SRC)/main.cc
	$(CPP) $(CFLAGS) -o $@ -c $^

$(OBJ)/compression/huffman/huffman.o: $(SRC)/compression/huffman/huffman.h $(SRC)/compression/huffman/node.h
	$(CPP) $(CFLAGS) -o $@ -c $(SRC)/compression/huffman/huffman.cc

$(OBJ)/base/bitstring.o: $(SRC)/base/bitstring.h
	$(CPP) $(CFLAGS) -o $@ -c $(SRC)/base/bitstring.cc

$(OBJ)/base/bitstring_test.o: $(SRC)/base/bitstring_test.cc
	$(CPP) $(CFLAGS) -o $@ -c $^

$(OBJ)/compression/huffman/huffman_test.o: $(SRC)/compression/huffman/huffman_test.cc
	$(CPP) $(CFLAGS) -o $@ -c $^

$(TEST)/bitstring: $(OBJ)/base/bitstring_test.o $(OBJ)/base/bitstring.o
	$(CPP) $(CFLAGS) -o $@ $^

$(TEST)/huffman: $(OBJ)/base/bitstring.o $(OBJ)/compression/huffman/huffman.o $(OBJ)/compression/huffman/huffman_test.o
	$(CPP) $(CFLAGS) -0 $@ $^

.PHONY: clean all test

