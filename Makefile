# vim: set noexpandtab :

CC      = gcc
CFLAGS  = -Werror -Wall -Wextra -pedantic -O0 -g
LDFLAGS = -rdynamic -ldl
TARGETS = c3qo \
          libblock.so

SRC = engine/common/src/main_inotify.c \
      engine/common/src/c3qo_signal.c  \
      engine/common/src/c3qo_socket.c
OBJ = $(SRC:.c=.o)

SHARED_SRC = block/hello/common/src/hello.c     \
             block/goodbye/common/src/goodbye.c \
             block/server_us_asnb/common/src/server_us_asnb.c \
             block/client_us_asnb/common/src/client_us_asnb.c \
             block/inotify_sb/common/src/inotify_sb.c
SHARED_OBJ = $(SHARED_SRC:.c=.o)



all : $(TARGETS)



### MAIN EXECUTABLE
#
c3qo : $(OBJ) 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

engine/common/src/main.o : engine/common/src/main.c
	$(CC) $(CFLAGS) -o $@ -c $<

engine/common/src/main_inotify.o : engine/common/src/main_inotify.c
	$(CC) $(CFLAGS) -o $@ -c $<

engine/common/src/c3qo_signal.o : engine/common/src/c3qo_signal.c
	$(CC) $(CFLAGS) -o $@ -c $<

engine/common/src/c3qo_socket.o : engine/common/src/c3qo_socket.c
	$(CC) $(CFLAGS) -o $@ -c $<
#



### BLOCKS LIBRARY
#

BLOCK_DEPENDS = block/block.h

libblock.so : $(SHARED_OBJ)
	$(CC) $(CFLAGS) -shared -o $@ $^

block/hello/common/src/hello.o : block/hello/common/src/hello.c $(BLOCK_DEPENDS)
	$(CC) $(CFLAGS) -fpic -o $@ -c $<

block/goodbye/common/src/goodbye.o : block/goodbye/common/src/goodbye.c $(BLOCK_DEPENDS)
	$(CC) $(CFLAGS) -fpic -o $@ -c $<

block/server_us_asnb/common/src/server_us_asnb.o : block/server_us_asnb/common/src/server_us_asnb.c $(BLOCK_DEPENDS)
	$(CC) $(CFLAGS) -fpic -o $@ -c $<

block/client_us_asnb/common/src/client_us_asnb.o : block/client_us_asnb/common/src/client_us_asnb.c $(BLOCK_DEPENDS)
	$(CC) $(CFLAGS) -fpic -o $@ -c $<

block/inotify_sb/common/src/inotify_sb.o : block/inotify_sb/common/src/inotify_sb.c $(BLOCK_DEPENDS)
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
	rm -rf block/inotify_sb/common/src/*.o

clean : clean-obj
	rm -rf $(TARGETS)
#

