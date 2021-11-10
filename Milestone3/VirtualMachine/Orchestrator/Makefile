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
ROOTDIR=../..

# Build rules for the SharedCommonCode directory
include $(ROOTDIR)/Common.mk

# Add the new include folders here
PYTHONINC = /usr/include/python3.8
INCLUDE += -isystem$(PYTHONINC)

INCLUDE+=-I$(shell realpath ./Include)
INCLUDE+=-I$(shell realpath ../../SharedCommonCode/Include)
INCLUDE+=-I$(shell realpath ../../WebService/RestApiPortal/Include)
INCLUDE+=-I$(shell realpath ../../WebService/SharedCommonCode/Include)
INCLUDE+=-I$(shell realpath ../../VirtualMachine/ComputationalDomainProcess/Include)
INCLUDE+=-I$(shell realpath ../../VirtualMachine/SharedComponents/Include)

# Add the new source directory here
SRCDIRS+=$(shell realpath ./Sources)
SRCDIRS+=$(shell realpath ../../SharedCommonCode/Sources)
SRCDIRS+=$(shell realpath ../../WebService/RestApiPortal/Sources)
SRCDIRS+=$(shell realpath ../../VirtualMachine/SharedComponents/Sources)

FINDFILES = $(wildcard $(DIR)/*.cpp)
SOURCES := $(foreach DIR,$(SRCDIRS),$(FINDFILES))
C_SOURCES := $(foreach DIR,$(SRCDIRS),$(C_FINDFILES))

OBJS=$(SOURCES:$(ABSROOTDIR)%.cpp=$(OBJDIR)%.o)
OBJS+=$(C_SOURCES:$(ABSROOTDIR)%.c=$(OBJDIR)%.o)

OUTPUT=$(ABSROOTDIR)/Binary/SAILPyAPI.so

all: $(OUTPUT)

$(OBJDIR)%.o: $(ABSROOTDIR)%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $^ -o $@

$(OBJDIR)%.o: $(ABSROOTDIR)%.c
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $^ -o $@

$(OUTPUT): $(OBJS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $^ -o $@ $(LDFLAGS) $(LDLIBS)
	@cp $@ ./sail/sail/

clean:
	rm -rf $(OUTPUT) $(OBJS)

cleanall:
	rm -rf $(ABSROOTDIR)/Binary
