SRC_FILES = $(wildcard src/*.c)
OBJ_FILES = $(addprefix obj/, $(notdir $(SRC_FILES:%.c=%.o)))
INC_FILES = $(wildcard inc/*.h)

CFLAGS = -std=c11 -Wall -Wextra -Werror -Wpedantic

all: LIBMX

LIBMX: $(OBJ_FILES)
	@ar rcs libmx.a $^

$(OBJ_FILES): obj 
obj:
	@mkdir -p $@
obj/%.o: src/%.c $(INC_FILES)
	clang $(CFLAGS) -c $< -o $@ -I inc

clean:
	@rm -rf obj $@
	@rm -rf $(OBJ_FILES) $@	

uninstall: clean
	@rm -rf libmx.a

reinstall: uninstall all

.PHONY: all uninstall clean reinstall
