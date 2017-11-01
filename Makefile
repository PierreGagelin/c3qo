# vim: set noexpandtab :

CC      = gcc
CFLAGS  = -Werror -Wall -Wextra -pedantic -O0 -g
LDFLAGS = -rdynamic -ldl
TARGETS = c3qo \
          libblock.so

SRC = engine/common/src/main.c        \
      engine/common/src/c3qo_signal.c \
      engine/common/src/c3qo_socket.c
OBJ = $(SRC:.c=.o)

SHARED_SRC = block/hello/common/src/hello.c     \
             block/goodbye/common/src/goodbye.c \
             block/server_us_asnb/common/src/server_us_asnb.c \
             block/client_us_asnb/common/src/client_us_asnb.c
SHARED_OBJ = $(SHARED_SRC:.c=.o)



all : $(TARGETS)



### MAIN EXECUTABLE
#
c3qo : $(OBJ) 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

engine/common/src/main.o : engine/common/src/main.c
	$(CC) $(CFLAGS) -o $@ -c $<

engine/common/src/c3qo_signal.o : engine/common/src/c3qo_signal.c
	$(CC) $(CFLAGS) -o $@ -c $<
#



### BLOCKS LIBRARY
#
libblock.so : $(SHARED_OBJ)
	$(CC) $(CFLAGS) -shared -o $@ $^

block/hello/common/src/hello.o : block/hello/common/src/hello.c
	$(CC) $(CFLAGS) -fpic -o $@ -c $<

block/goodbye/common/src/goodbye.o : block/goodbye/common/src/goodbye.c
	$(CC) $(CFLAGS) -fpic -o $@ -c $<

block/server_us_asnb/common/src/server_us_asnb.o : block/server_us_asnb/common/src/server_us_asnb.c
	$(CC) $(CFLAGS) -fpic -o $@ -c $<

block/client_us_asnb/common/src/client_us_asnb.o : block/client_us_asnb/common/src/client_us_asnb.c
	$(CC) $(CFLAGS) -fpic -o $@ -c $<
#



### CLEANING
#
clean-obj :
	rm -rf engine/common/src/*.o
	rm -rf block/hello/common/src/*.o
	rm -rf block/goodbye/common/src/*.o
	rm -rf block/server_us_asnb/common/src/*.o
	rm -rf block/client_us_asnb/common/src/*.o

clean : clean-obj
	rm -rf $(TARGETS)
#

