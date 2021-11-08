BUILD_MODE?=debug
CXXFLAGS=-std=c++17 -fstack-protector -DUNIX -DOS_UNIX -Wall -g -MMD -MP -fPIC
LDFLAGS = -Xlinker -export-dynamic -shared
LDLIBS = -lSharedCommonCode -lpthread -ldl -luuid -lrt -lcrypto -lssl -lutil -lm -lstdc++fs -lpython3.8 -lcurl

ifeq ($(BUILD_MODE), debug)
	CXXFLAGS+= -ggdb -O0
else
	CXXFLAGS+= -O3
endif

# The Root directory containing all the code
ROOTDIR=../..

# Build rules for the SharedCommonCode directory
include $(ROOTDIR)/Common.mk

# Add the new include folders here
PYTHONINC = /usr/include/python3.8
INCLUDE += -isystem$(PYTHONINC)

INCLUDE+=-I$(shell realpath ./Include)
INCLUDE+=-I$(shell realpath ../../WebService/SharedCommonCode/Include)
INCLUDE+=-I$(shell realpath ../../WebService/RestApiPortal/Include)
INCLUDE+=-I$(shell realpath ../../VirtualMachine/ComputationalDomainProcess/Include)
INCLUDE+=-I$(shell realpath ../../VirtualMachine/SharedComponents/Include)

# Add the new source directory here
SRCDIRS+=$(shell realpath ./Sources)
SRCDIRS+=$(shell realpath ../../WebService/RestApiPortal/Sources)
SRCDIRS+=$(shell realpath ../../VirtualMachine/SharedComponents/Sources)

FINDFILES = $(wildcard $(DIR)/*.cpp)
SOURCES := $(foreach DIR,$(SRCDIRS),$(FINDFILES))

OBJS=$(SOURCES:$(ABSROOTDIR)%.cpp=$(OBJDIR)%.o)

OUTPUT=$(ABSROOTDIR)/Binary/SAILPyAPI.so

.PHONY: all, clean, cleanall

all: $(OUTPUT)

$(OBJDIR)%.o: $(ABSROOTDIR)%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $^ -o $@

$(OUTPUT): $(OBJS) SharedCommonCode
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(filter %.o,$^) -o $@ $(LDFLAGS) $(LDLIBS)
	@cp $@ ./sail/sail/

cleanall:
	rm -rf $(ABSROOTDIR)/Binary
