VPATH	=	../../common
CFLAGS	=	-I./../../common/
objects = array.o buffer.o list.o fdset.o mutex.o \
	rwlist.o socketlibfunction.o stack.o systemlib.o \
	thread.o timeaction.o typepool.o
common	:	$(objects)
	gcc -shared $(objects) -o common.so
%.o	:	%.c
	gcc -fPIC $(CFLAGS)  -c $< 
