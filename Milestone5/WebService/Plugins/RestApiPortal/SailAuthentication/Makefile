BUILD_MODE?=debug
CXXFLAGS=-std=c++17 -fPIC -fstack-protector -DUNIX -DOS_UNIX
LDFLAGS=-lSharedCommonCode -pthread -ldl -luuid -lcrypto -lssl -lstdc++fs -lssl -lmongocxx -lbsoncxx -lcurl

ifeq ($(BUILD_MODE), debug)
	CXXFLAGS+= -ggdb
else
	CXXFLAGS+= -O3
endif

# The Root directory containing all the code
ROOTDIR=../../../..
ABSROOTDIR=$(shell realpath $(ROOTDIR))
OBJDIR=$(ABSROOTDIR)/Binary/Objects

# Build rules for the SharedCommonCode directory
include $(ROOTDIR)/Common.mk

# Add the new include folders here
INCLUDE+=-I$(shell realpath ./Include)
INCLUDE+=-I$(shell realpath ../../../../InternalTools/CryptographicTools/Include)
INCLUDE+=-I$(shell realpath ../../../../WebService/SharedCommonCode/Include)
INCLUDE+=-I$(shell realpath ../../../../WebService/Plugins/RestApiPortal/CryptographicKeyManagement)
INCLUDE+=-I$(shell realpath ../../../../WebService/RestApiPortal/Include)

# Includes for MongoDB C++ driver
INCLUDE+=-I/usr/local/include/mongocxx/v_noabi -I/usr/local/include/libmongoc-1.0 -I/usr/local/include/bsoncxx/v_noabi -I/usr/local/include/libbson-1.0 -L/usr/local/lib

# Add the new source directory here
SRCDIRS+=$(shell realpath ./Sources)
SRCDIRS+=$(shell realpath ../../../../WebService/RestApiPortal/Sources)

FINDFILES = $(wildcard $(DIR)/*.cpp)
SOURCES := $(foreach DIR,$(SRCDIRS),$(FINDFILES))
SOURCES += $(shell realpath ../../../../InternalTools/CryptographicTools/Sources/CryptoUtils.cpp)
SOURCES += $(shell realpath ../../../../WebService/Plugins/RestApiPortal/CryptographicKeyManagement/CryptographicEngine.cpp)
SOURCES += $(shell realpath ../../../../WebService/Plugins/RestApiPortal/CryptographicKeyManagement/CryptographicKey.cpp)
SOURCES += $(shell realpath ../../../../WebService/Plugins/RestApiPortal/CryptographicKeyManagement/CryptographyHelperFunctions.cpp)

OBJS=$(SOURCES:$(ABSROOTDIR)%.cpp=$(OBJDIR)%.o)
OUTPUT=$(ABSROOTDIR)/Binary/SharedLibraries/RestApiPortal/lib$(notdir $(shell pwd)).so

.PHONY: all, clean

all: $(OUTPUT)

$(OBJDIR)%.o: $(ABSROOTDIR)%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $^ -o $@

$(OUTPUT): $(OBJS) SharedCommonCode
	@mkdir -p $(@D)
	$(CXX) -shared $(CXXFLAGS) $(INCLUDE) $(filter %.o,$^) -o $@ $(LDFLAGS)

clean:
	rm -rf $(OBJS) $(OUTPUT) $(LOCAL_BIN)
