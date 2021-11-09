BUILD_MODE?=debug
CXXFLAGS=-std=c++17 -fstack-protector -DUNIX -DOS_UNIX -Wall -g -MMD -MP -fPIC
LDFLAGS = -Xlinker -export-dynamic
LDLIBS = -lSharedCommonCode -lpthread -ldl -luuid -lrt -lcrypto -lssl -lutil -lm -lstdc++fs -lpython3.8 -lcurl

ifeq ($(BUILD_MODE), debug)
	CXXFLAGS+= -ggdb -O0
else
	CXXFLAGS+= -O3
endif

# The Root directory containing all the code
ROOTDIR=../..

include $(ROOTDIR)/Common.mk

# Add the new include folders here
PYTHONINC:= /usr/include/python3.8
INCLUDE += -isystem$(PYTHONINC)

INCLUDE+=-I$(shell realpath ./Include)
INCLUDE+=-I$(shell realpath ../SharedComponents/Include)
INCLUDE+=-I$(shell realpath ../../EndPointTools/ResearcherInterface/Utils/Include)
INCLUDE+=-I$(shell realpath ../DataDomainProcess)

# Add the new source directory here
SRCDIRS+=$(shell realpath ./Sources)
SRCDIRS+=$(shell realpath ../../EndPointTools/ResearcherInterface/Utils/Sources)
SRCDIRS+=$(shell realpath ../SharedComponents/Sources)

SOURCES := $(foreach DIR,$(SRCDIRS),$(FINDFILES))

OBJS=$(SOURCES:$(ABSROOTDIR)%.cpp=$(OBJDIR)%.o)

OUTPUT=$(ABSROOTDIR)/Binary/$(notdir $(shell pwd))

.PHONY: all, clean, cleanall

all: $(OUTPUT)

$(OBJDIR)%.o: $(ABSROOTDIR)%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $^ -o $@

$(OUTPUT): $(OBJS) SharedCommonCode
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(filter %.o,$^) -o $@ $(LDFLAGS) $(LDLIBS)

clean:
	rm -rf $(OUTPUT) $(OBJS)

cleanall:
	rm -rf $(ABSROOTDIR)/Binary
