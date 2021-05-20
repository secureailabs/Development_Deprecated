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

# Add the new include folders here
INCLUDE+=-I$(shell realpath .)
INCLUDE+=-I$(shell realpath ../../SharedCommonCode/Include)

# Add the new source directory here
SRCDIRS+=$(shell realpath .)
SRCDIRS+=$(shell realpath ../../SharedCommonCode/Sources)

FINDFILES = $(wildcard $(DIR)/*.cpp)
SOURCES := $(foreach DIR,$(SRCDIRS),$(FINDFILES))

OBJS=$(SOURCES:$(ABSROOTDIR)%.cpp=$(OBJDIR)%.o)
OUTPUT=$(ABSROOTDIR)/Binary/$(notdir $(shell pwd))

all: $(OUTPUT)
	@cp VmConfig.json $(ABSROOTDIR)/Binary/
	@cp VmFromImage.json $(ABSROOTDIR)/Binary/
	@cp NetworkInterface.json $(ABSROOTDIR)/Binary/
	@cp PublicIpAddress.json $(ABSROOTDIR)/Binary/
	@cp ResourceGroup.json $(ABSROOTDIR)/Binary/
	@cp VirtualNetwork.json $(ABSROOTDIR)/Binary/
	@cp Telco1.csvp $(ABSROOTDIR)/Binary/
	@cp Telco2.csvp $(ABSROOTDIR)/Binary/
	@cp Telco.csvp $(ABSROOTDIR)/Binary/

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
