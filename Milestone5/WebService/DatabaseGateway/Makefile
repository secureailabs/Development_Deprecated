BUILD_MODE?=debug
CXXFLAGS=-std=c++17 -fPIC -fstack-protector -DUNIX -DOS_UNIX -rdynamic
LDFLAGS=-lpthread -ldl -luuid -lrt -lcrypto -lssl -lstdc++fs -lssl -lcurl

ifeq ($(BUILD_MODE), debug)
	CXXFLAGS+= -ggdb
else
	CXXFLAGS+= -O3
endif

# The Root directory containing all the code
ROOTDIR=../..

include $(ROOTDIR)/Common.mk

# Add the new include folders here
INCLUDE+=-I$(shell realpath ./Include)

# Add the new source directory here
SRCDIRS+=$(shell realpath ./Sources)

SOURCES := $(foreach DIR,$(SRCDIRS),$(FINDFILES))
C_SOURCES := $(foreach DIR,$(SRCDIRS),$(C_FINDFILES))

OBJS=$(SOURCES:$(ABSROOTDIR)%.cpp=$(OBJDIR)%.o)
OBJS+=$(C_SOURCES:$(ABSROOTDIR)%.c=$(OBJDIR)%.o)
OUTPUT=$(ABSROOTDIR)/Binary/$(notdir $(shell pwd))

all: $(OUTPUT)

$(OBJDIR)%.o: $(ABSROOTDIR)%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $^ -o $@

$(OBJDIR)%.o: $(ABSROOTDIR)%.c
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $^ -o $@


$(OUTPUT): $(OBJS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf $(OUTPUT) $(OBJS)

cleanall:
	rm -rf $(ABSROOTDIR)/Binary
