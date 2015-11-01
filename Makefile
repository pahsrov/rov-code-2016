CC = g++
CXXFLAGS = -Wall -Wextra --std=c++11 $(OPTFLAGS)
LFLAGS = -lm
DEVFLAGS = -g

#do a search for all c files in src/
SOURCES = $(wildcard src/**/*.cpp src/*.cpp src/**/*.c src/*.c)

# turn c files into object files
COBJECTS = $(patsubst %.c, %.o, $(SOURCES))
OBJECTS = $(patsubst %.cpp, %.o, $(COBJECTS))


#find all header files
HEADERS = $(wildcard include/*.hpp include/**/*.hpp include/**/*.h include/*.h)
NAME = rov

all: $(HEADERS) $(NAME)

$(NAME) : $(OBJECTS)
	@echo "OBJECTS "$(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

dev : CFLAGS =-Wall -Wextra --std=c++11 -g $(OPTFLAGS)
dev : all
