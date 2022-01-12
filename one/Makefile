CC		= g++

#INCLUDE = -I../include
#LIBIB= libib.a

INLCUDE=
CFLAGS += $(INCLUDE)
CFLAGS += -c -g
CFLAGS += -D_POSIX_C_SOURCE -D_GNU_SOURCE
#CFLAGS += -std=gnu99 -Wall -Werror

#LDFLAGS += -L../lib
#LDFLAGS += -lpthread -libverbs -lrdmacm -l:$(LIBIB)
LDFLAGS=

TARGETS = main

all: $(TARGETS)

main: main.o user.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: clean
clean:
	rm -f *.o $(TARGETS) cscope.out tags

.PHONY: cscope
cscope:
	cscope -b -R
