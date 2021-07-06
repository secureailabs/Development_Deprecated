BUILD_MODE?=debug
CXXFLAGS=-std=c++17 -fPIC -fstack-protector -DUNIX -DOS_UNIX
LDFLAGS=-pthread -ldl -luuid -lcrypto -lssl -lstdc++fs -lssl -lcurl

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
INCLUDE+=-I$(shell realpath ../../../../WebService/RestApiPortal/Include)
INCLUDE+=-I$(shell realpath ../../../../SharedCommonCode/Include)

# Add the new source directory here
SRCDIRS+=$(shell realpath ./)
SRCDIRS+=$(shell realpath ../../../../WebService/RestApiPortal/Sources)
SRCDIRS+=$(shell realpath ../../../../SharedCommonCode/Sources)

FINDFILES = $(wildcard $(DIR)/*.cpp)
SOURCES := $(foreach DIR,$(SRCDIRS),$(FINDFILES))

OBJS=$(SOURCES:$(ABSROOTDIR)%.cpp=$(OBJDIR)%.o)
LOCAL_BIN=$(ABSROOTDIR)/WebService/SharedLibraries/RestApiPortal/lib$(notdir $(shell pwd)).so
OUTPUT=$(ABSROOTDIR)/Binary/SharedLibraries/RestApiPortal/lib$(notdir $(shell pwd)).so

all: $(OUTPUT)

$(OUTPUT): $(LOCAL_BIN)
	@mkdir -p $(@D)
	@cp $^ $@

$(OBJDIR)%.o: $(ABSROOTDIR)%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $^ -o $@

$(LOCAL_BIN): $(OBJS)
	@mkdir -p $(@D)
	$(CXX) -shared $(CXXFLAGS) $(INCLUDE) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf $(OBJS) $(OUTPUT) $(LOCAL_BIN)
