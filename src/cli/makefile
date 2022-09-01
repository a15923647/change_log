target:=runrun
PP:=g++
cc:=gcc
CPPFLAGS:= -g -std=c++17 -I ../include/
LINKFLAGS:= -lcrypto -lpthread -ldl
source:=$(shell find . -maxdepth 1 -type f -name "*.cpp")
objs:=$(patsubst %.cpp, %.o, $(source))

deps:=$(patsubst %.o, %.d, $(objs))
-include $(deps)
DEPFLAGS = -MMD -MF $(@:.o=.d)

all: $(target)

runrun: $(objs) sqlite3.o
	$(PP) -o $@ $^ $(LINKFLAGS)

sqlite3 : sqlite3.c
	$(cc) -g -o sqlite3.c -o sqlite3.o -lpthread -ldl

%.o: %.cpp
	$(PP) $(CPPFLAGS) -c $< $(DEFFLAGS)

clean:
	rm -f $(target) $(objs) $(deps)
