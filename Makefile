CC = g++
CXXFLAGS = -Wall -Wextra --std=c++11 $(OPTFLAGS)
LDFLAGS = -lm
DEVFLAGS = -g

#do a search for all c files in src/
SOURCES = $(wildcard src/**/*.cpp src/*.cpp src/**/*.c src/*.c)

# turn c files into object files
COBJECTS = $(patsubst %.c, %.o, $(SOURCES))
OBJECTS = $(patsubst %.cpp, %.o, $(COBJECTS))

#find all header files
HEADERS = $(wildcard include/*.hpp include/**/*.hpp include/**/*.h include/*.h)
NAME = rov
# DEPS = $(OBJECTS:.o=.d)

# include $(HEADERS)

all: $(HEADERS) $(NAME)

$(NAME) : $(OBJECTS)
	$(CC) $(CFLAGS) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

dev : CXXFLAGS += -g
dev : all

# %.d: %.o
# 	@set -e; rm -f $@;
# 	$(CC) -M $(CXXFLAGS) $(CFLAGS) $<
# 	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@ > $@;

clean:
	rm -f src/*.o src/*.d
