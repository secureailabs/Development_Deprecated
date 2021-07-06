BUILD_MODE?=debug
CXXFLAGS=-std=c++17 -fstack-protector -DUNIX -DOS_UNIX -fPIC
LDFLAGS=-lpthread -ldl -luuid -lrt -lcrypto -lssl -lcurl

ifeq ($(BUILD_MODE), debug)
	CXXFLAGS+= -ggdb
else
	CXXFLAGS+= -O3
endif

# The Root directory containing all the code
ROOTDIR=../..
ABSROOTDIR=$(shell realpath $(ROOTDIR))
OBJDIR=$(ABSROOTDIR)/Binary/Objects

# Location of the Initializertool
INITIALIZER_TOOL=$(shell realpath ../../EndPointTools/InitializerTool)

# Add the new include folders here
INCLUDE+=-I$(shell realpath .)
INCLUDE+=-I$(shell realpath ../../SharedCommonCode/Include)
INCLUDE+=-I$(INITIALIZER_TOOL)

# Add the new source directory here
SRCDIRS+=$(shell realpath .)
SRCDIRS+=$(shell realpath ../../SharedCommonCode/Sources)

FINDFILES = $(wildcard $(DIR)/*.cpp)
SOURCES := $(foreach DIR,$(SRCDIRS),$(FINDFILES))
SOURCES+=$(INITIALIZER_TOOL)/Azure.cpp

OBJS=$(SOURCES:$(ABSROOTDIR)%.cpp=$(OBJDIR)%.o)
OUTPUT=$(ABSROOTDIR)/Binary/$(notdir $(shell pwd))

all: $(OUTPUT)
	@echo $(INITIALIZER_TOOL)
	@cp $(INITIALIZER_TOOL)/VmConfig.json $(ABSROOTDIR)/Binary/
	@cp $(INITIALIZER_TOOL)/VmFromImage.json $(ABSROOTDIR)/Binary/
	@cp $(INITIALIZER_TOOL)/NetworkInterface.json $(ABSROOTDIR)/Binary/
	@cp $(INITIALIZER_TOOL)/PublicIpAddress.json $(ABSROOTDIR)/Binary/
	@cp $(INITIALIZER_TOOL)/ResourceGroup.json $(ABSROOTDIR)/Binary/
	@cp $(INITIALIZER_TOOL)/VirtualNetwork.json $(ABSROOTDIR)/Binary/

$(OBJDIR)%.o: $(ABSROOTDIR)%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $^ -o $@

$(OUTPUT): $(OBJS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf $(OUTPUT) $(OBJS)

cleanall:
	rm -rf $(ABSROOTDIR)/Binary
