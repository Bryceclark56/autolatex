CC 		= clang
CFLAGS 	= -Wall -Werror -stdlib=musl
CFLAGSDEBUG = -fsanitize=memory -fno-omit-frame-pointer -ggdb -D DEBUG
OUTPUT 	= build/autolatex
SRC 	= src/autolatex.c


.PHONY: all
all: autolatex

.PHONY: autolatex
autolatex: build $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUTPUT)

.PHONY: debug
debug: build $(SRC)
	$(CC) $(CFLAGS) $(CFLAGSDEBUG) $(SRC) -o $(OUTPUT)

.PHONY: clean
clean:
	rm -r build

build:
	mkdir -p build
