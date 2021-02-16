BUILD_MODE?=debug
CXXFLAGS=-std=c++17 -fPIC -fstack-protector -DUNIX -DOS_UNIX
LDFLAGS=-pthread -ldl -luuid -lcrypto -lssl -lstdc++fs -lssl -lmongocxx -lbsoncxx

ifeq ($(BUILD_MODE), debug)
	CXXFLAGS+= -ggdb
else
	CXXFLAGS+= -O3
endif

# The Root directory containing all the code
ROOTDIR=../../../../
ABSROOTDIR=$(shell realpath $(ROOTDIR))
OBJDIR=$(ABSROOTDIR)/Binary/Objects

# Add the new include folders here
INCLUDE+=-I$(shell realpath ./Include)
INCLUDE+=-I$(shell realpath ../../../../InternalTools/CryptographicTools/Include)
INCLUDE+=-I$(shell realpath ../../../../WebService/RestApiPortal/Include)
INCLUDE+=-I$(shell realpath ../../../../SharedCommonCode/Include)
# Includes for MongoDB C++ driver
INCLUDE+=-I/usr/local/include/mongocxx/v_noabi -I/usr/local/include/libmongoc-1.0 -I/usr/local/include/bsoncxx/v_noabi -I/usr/local/include/libbson-1.0 -L/usr/local/lib

# Add the new source directory here
SRCDIRS+=$(shell realpath ./Sources)
SRCDIRS+=$(shell realpath ../../../../WebService/RestApiPortal/Sources)
SRCDIRS+=$(shell realpath ../../../../SharedCommonCode/Sources)

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
	$(CXX) -shared $(CXXFLAGS) $(INCLUDE) $^ -o ../../../SharedLibraries/RestApiPortal/libAudiLogManager.so $(LDFLAGS)

clean:
	rm -rf $(OUTPUT) $(OBJS)
