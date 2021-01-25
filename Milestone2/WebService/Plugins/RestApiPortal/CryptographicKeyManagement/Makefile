BUILD_MODE?=debug
CXXFLAGS=-std=c++17 -fPIC -fstack-protector -DUNIX -DOS_UNIX
LDFLAGS=-pthread -ldl -luuid -lcrypto -lssl -lstdc++fs -lssl

ifeq ($(BUILD_MODE), debug)
	CXXFLAGS+= -ggdb
else
	CXXFLAGS+= -O3
endif

# The Root directory containing all the code
ROOTDIR=../../../..
ABSROOTDIR=$(shell realpath $(ROOTDIR))
OBJDIR=$(ABSROOTDIR)/Binary/Objects

# Add the new include folders here
INCLUDE+=-I$(shell realpath ./)
INCLUDE+=-I$(shell realpath ../../../../WebService/ApiPortal/Include)
INCLUDE+=-I$(shell realpath ../../../../SharedCommonCode/Include)
INCLUDE+=-I$(shell realpath ../../../../VirtualMachine/SharedComponents/Include)
INCLUDE+=-I$(shell realpath ../../../../OlderVersionOfTlsLibrary)

# Add the new source directory here
SRCDIRS+=$(shell realpath ./)
SRCDIRS+=$(shell realpath ../../../../WebService/ApiPortal/Sources)
SRCDIRS+=$(shell realpath ../../../../SharedCommonCode/Sources)
SRCDIRS+=$(shell realpath ../../../../VirtualMachine/SharedComponents/Sources)
SRCDIRS+=$(shell realpath ../../../../OlderVersionOfTlsLibrary)

FINDFILES = $(wildcard $(DIR)/*.cpp)
SOURCES := $(foreach DIR,$(SRCDIRS),$(FINDFILES))

OBJS=$(SOURCES:$(ABSROOTDIR)%.cpp=$(OBJDIR)%.o)
OUTPUT=plugin

all: $(OUTPUT)

$(OBJDIR)%.o: $(ABSROOTDIR)%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $^ -o $@

$(OUTPUT): $(OBJS)
	@mkdir -p $(@D)
	$(CXX) -shared $(CXXFLAGS) $(INCLUDE) $^ -o ../../../SharedLibraries/RestApiPortal/libCryptoManagement.so $(LDFLAGS)

clean:
	rm -rf $(OUTPUT) $(OBJS)
