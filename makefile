target:=runrun
PP:=g++
CPPFLAGS:= -g -lcrypto -lpthread -std=c++17
LINKFLAGS:= -lcrypto -lpthread
source:=$(shell find . -maxdepth 1 -type f -name "*.cpp")
objs:=$(patsubst %.cpp, %.o, $(source))

deps:=$(patsubst %.o, %.d, $(objs))
-include $(deps)
DEPFLAGS = -MMD -MF $(@:.o=.d)

all: $(target)

runrun: $(objs)
	$(PP) -o $@ $^ $(LINKFLAGS)

%.o: %.cpp
	$(PP) $(CPPFLAGS) -c $< $(DEFFLAGS)

clean:
	rm -f $(target) $(objs) $(deps)
