VPATH	=	../../common ../../server-model01
CFLAGS	=	-I./../../common/ -I./../../server-model01
objects = array.o buffer.o list.o fdset.o mutex.o \
	rwlist.o socketlibfunction.o stack.o systemlib.o \
	thread.o timeaction.o typepool.o threadpool.o \
	server.o epollserver.o
TARGET	=	common.so
common	:	$(objects)
	gcc -shared $(objects) -o $(TARGET) 
clean	:	   
	rm $(objects) -rf
	rm $(TARGET) -rf 
%.o	:	%.c
	gcc -fPIC $(CFLAGS)  -c $< 
