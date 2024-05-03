CC=gcc
CFLAGS=`pkg-config --cflags cairo`
LIBS=`pkg-config --libs cairo` -lm
OBJS=lsystem.o lsys.o turtle_stack.o turtle.o vector.o utils.o viewport.o coordinate_system.o cache.o timer.o

lsystem: $(OBJS)
	$(CC) -o lsystem $(OBJS) $(LIBS)

lsystem.o: lsystem.c turtle_stack.h
	$(CC) -c lsystem.c $(CFLAGS)

lsys.o: lsys.c lsys.h
	$(CC) -c lsys.c $(CFLAGS)

cache.o: cache.c cache.h
	$(CC) -c cache.c $(CFLAGS)

coordinate_system.o: coordinate_system.c coordinate_system.h
	$(CC) -c coordinate_system.c $(CFLAGS)

turtle_stack.o: turtle_stack.c turtle_stack.h
	$(CC) -c turtle_stack.c $(CFLAGS)

turtle.o: turtle.c turtle.h
	$(CC) -c turtle.c $(CFLAGS)

timer.o: timer.c timer.h
	$(CC) -c timer.c $(CFLAGS)

utils.o: utils.c utils.h
	$(CC) -c utils.c $(CFLAGS)

vector.o: vector.c vector.h
	$(CC) -c vector.c $(CFLAGS)

viewport.o: viewport.c viewport.h
	$(CC) -c viewport.c $(CFLAGS)

clean:
	rm -f lsystem $(OBJS)