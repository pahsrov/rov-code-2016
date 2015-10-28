CC = g++
CFLAGS = -Wall -Wextra --std=c++11 $(OPTFLAGS)
LFLAGS = -lm
DEVFLAGS = -g

#do a search for all c files in src/
SOURCES = $(wildcard src/**/*.cpp src/*.cpp src/**/*.c src/*.c)

# turn c files into object files
OBJECTS = $(patsubst %.c, %.o, $(SOURCES))

#find all header files
HEADERS = $(wildcard include/*.hpp include/**/*.hpp include/**/*.h include/*.h)
#directory where object files are stored
NAME = rov

all: $(NAME)

$(NAME) : $(OBJECTS) $(HEADERS) $(SOURCES)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LFLAGS)

dev : CFLAGS =-Wall -Wextra --std=c++11 -g $(OPTFLAGS)
dev : all
