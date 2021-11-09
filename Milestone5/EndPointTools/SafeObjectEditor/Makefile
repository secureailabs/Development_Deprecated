BUILD_MODE?=debug
CXXFLAGS=-std=c++17 -fstack-protector -DUNIX -DOS_UNIX -Wall -g -MMD -MP -fPIC
LDFLAGS = -Xlinker -export-dynamic -shared
LDLIBS = -lpthread -ldl -luuid -lrt -lcrypto -lssl -lutil -lm -lstdc++fs -lpython3.8 -lcurl

ifeq ($(BUILD_MODE), debug)
	CXXFLAGS+= -ggdb -O0
else
	CXXFLAGS+= -O3
endif

# The Root directory containing all the code
ROOTDIR=.
ABSROOTDIR=$(shell realpath $(ROOTDIR))
OBJDIR=$(ABSROOTDIR)/Binary/Objects

# Add the new include folders here
PYTHONINC = /usr/include/python3.8
INCLUDE := -isystem$(PYTHONINC)

INCLUDE+=-I$(shell realpath ../../SharedCommonCode/Include)

# Add the new source directory here
SRCDIRS+=$(shell realpath .)
SRCDIRS+=$(shell realpath ../../SharedCommonCode/Sources)

FINDFILES = $(wildcard $(DIR)/*.cpp)
SOURCES := $(foreach DIR,$(SRCDIRS),$(FINDFILES))

OBJS=$(SOURCES:$(ABSROOTDIR)%.cpp=$(OBJDIR)%.o)
OUTPUT=SafeObjectAPI.so

all: $(OUTPUT)

$(OBJDIR)%.o: $(ABSROOTDIR)%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $^ -o $@

$(OUTPUT): $(OBJS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $^ -o $@ $(LDFLAGS) $(LDLIBS)

cleanall:
	rm -rf $(ABSROOTDIR)/Binary
