CC 		= clang
CFLAGS 	= -Wall -Werror -stdlib=musl
OUTPUT 	= build/autolatex
SRC 	= src/autolatex.c


#all:build

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUTPUT)
