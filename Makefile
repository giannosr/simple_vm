.PHONY: dev clean distclean default

CC = gcc
CFLAGS = -O3 -march=native -std=gnu17
DEV_FLAGS = -Wall -Wextra -Werror -ggdb -Og -std=gnu17 -fsanitize=undefined \
  -fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract


vm: vm.o gc.o
	$(CC) $(CFLAGS) -o $@ $^

vm.o: vm.h gc.h
gc.o: vm.h gc.h

dev: CFLAGS := $(DEV_FLAGS)
dev: vm

clean:
	$(RM) vm.o gc.o

distclean: clean
	$(RM) vm