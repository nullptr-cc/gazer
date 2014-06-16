NAME = gazer
CPP = g++
LINKER = g++
CPPFLAGS = -c -std=c++11
LFLAGS =
LIBS = -lconfig++
OBJECTS = main.o daemonize.o signal_handlers.o workers.o

all: ${OBJECTS}
	${LINKER} ${OBJECTS} -o ${NAME} ${LIBS}

main.o:
	${CPP} ${CPPFLAGS} main.cpp

daemonize.o:
	${CPP} ${CPPFLAGS} daemonize.cpp

signal_handlers.o:
	${CPP} ${CPPFLAGS} signal_handlers.cpp

workers.o:
	${CPP} ${CPPFLAGS} workers.cpp

clean:
	rm -f *.o ${NAME}
