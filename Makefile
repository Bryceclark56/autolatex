CC 		= clang
CFLAGS 	= -Wall -Werror -stdlib=musl
OUTPUT 	= build/autolatex
SRC 	= src/autolatex.c


all: build $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUTPUT)

build:
	mkdir build
