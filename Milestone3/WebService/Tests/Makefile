BUILD_MODE?=debug
CXXFLAGS=-std=c++17 -fPIC -fstack-protector -DUNIX -DOS_UNIX -fPIC
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
INCLUDE+=-I$(shell realpath ../../EndPointTools/RestPortalTester/Sources)
INCLUDE+=-I$(shell realpath ../../WebService/RestApiPortal/Include)

# Add the new source directory here
SRCDIRS+=$(shell realpath ./Sources)

C_SOURCES := $(foreach DIR,$(SRCDIRS),$(C_FINDFILES))
SOURCES := $(foreach DIR,$(SRCDIRS),$(FINDFILES))
SOURCES+=$(shell realpath ../../EndPointTools/RestPortalTester/Sources/InteractiveClient.cpp)
SOURCES+=$(shell realpath ../../EndPointTools/RestPortalTester/Sources/GuiTerminal.cpp)

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
