# build shared library with -fPIC, -shared
CXXFLAGS   = # -g -O3 -fPIC # CXXFLAGS for .cpp
CPPFLAGS = #-MMD -MP -I../foo
LDFLAGS  = # -L../foo -Wl,-rpath,'$$ORIGIN/../foo' # -shared
LDLIBS   = -lpthread
CC      = $(CXX) # link with CXX for .cpp

CPPFLAGS += -I/home/ljh/Downloads/boost_1_81_0
LDFLAGS += -L/home/ljh/Downloads/boost_1_81_0/stage/lib

# make # NDEBUG=1
ifdef NDEBUG
	CPPFLAGS += -DNDEBUG
	CXXFLAGS += -O3 # .cpp
else
	CXXFLAGS += -g # .cpp
	LDFLAGS += -fsanitize=address
endif

all: server client

# target name is basename of one of the source files
# main : $(patsubst %.c,%.o,$(wildcard *.c)) # .cpp
server : $(patsubst %.cpp,%.o,server.cpp) # .cpp
client : $(patsubst %.cpp,%.o,client.cpp) # .cpp
-include *.d
clean : ; -rm -fr *.o *.d server client
.PHONY : clean
