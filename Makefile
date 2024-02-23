# Makefile for the smash program
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Werror -pedantic-errors -DNDEBUG
CXXLINK = $(CXX)
OBJS = smash.o commands.o
RM = rm -f
# Creating the  executable
smash: $(OBJS)
	$(CXXLINK) -o smash $(OBJS)
# Creating the object files
commands.o: commands.cpp commands.h
smash.o: smash.cpp commands.h
# Cleaning old files before new make
clean:
	$(RM) $(TARGET) *.o *~ "#"* core.* smash
