SHARED_COMMON_CXXFLAGS=-std=c++17 -fPIC -fstack-protector -DUNIX -DOS_UNIX

BUILD_MODE?=debug
ifeq ($(BUILD_MODE), debug)
	SHARED_COMMON_CXXFLAGS+= -ggdb
else
	SHARED_COMMON_CXXFLAGS+= -O3
endif

ROOTDIR:=../

ABSROOTDIR=$(shell realpath $(ROOTDIR))
OBJDIR=$(ABSROOTDIR)/Binary/Objects
LIB_WORKSPACE_DIR = $(OBJDIR)/LibWorkspace

FINDFILES = $(wildcard $(DIR)/*.cpp)

# Add the new include folders here
SHARED_COMMON_INCLUDE :=-I$(shell realpath $(ABSROOTDIR)/SharedCommonCode/Include)
SHARED_COMMON_INCLUDE +=-I$(shell realpath $(ABSROOTDIR)/ThirdPartyLibraries/LzmaSdk/Include)
SHARED_COMMON_SRCDIRS :=$(shell realpath $(ABSROOTDIR)/SharedCommonCode/Sources)

SHARED_COMMON_SOURCES := $(foreach DIR,$(SHARED_COMMON_SRCDIRS),$(FINDFILES))

SHARED_COMMON_OBJS=$(SHARED_COMMON_SOURCES:$(ABSROOTDIR)%.cpp=$(OBJDIR)%.o)
SHARED_COMMON_LIBRARY=$(ABSROOTDIR)/Binary/StaticLibraries/libSharedCommonCode.a
SAIL_SHARED_CODE=$(ABSROOTDIR)/Binary/StaticLibraries/libSailHelpers.a
LZMA_LIBRARY=$(ABSROOTDIR)/Binary/StaticLibraries/liblzma-sail.a

.PHONY: all, clean

all: $(SHARED_COMMON_LIBRARY)

$(OBJDIR)%.o: $(ABSROOTDIR)%.cpp
	@mkdir -p $(@D)
	$(CXX) $(SHARED_COMMON_CXXFLAGS) $(SHARED_COMMON_INCLUDE) -c $^ -o $@

$(LZMA_LIBRARY):
	@make -C $(ABSROOTDIR)/ThirdPartyLibraries/LzmaSdk all

# We generate the library first as _solo so we can combine all static libraries into one with
# the intended target name
$(SAIL_SHARED_CODE): $(SHARED_COMMON_OBJS)
	@mkdir -p $(@D)
	@ar rcs $@ $^

# Combine both generated archives into one
# We extract the object files from each individual static library, then re-combine
$(SHARED_COMMON_LIBRARY): $(SAIL_SHARED_CODE) $(LZMA_LIBRARY)
	@mkdir -p $(LIB_WORKSPACE_DIR)
	@($(foreach StaticLib, $(filter %.a,$^), ar -x --output $(LIB_WORKSPACE_DIR) $(StaticLib);))
	@ar rcs $@ $(LIB_WORKSPACE_DIR)/*.o
	@rm $(LIB_WORKSPACE_DIR)/*.o
	@rmdir $(LIB_WORKSPACE_DIR)

clean:
	@rm $(SHARED_COMMON_LIBRARY) $(SHARED_COMMON_OBJS)
	@make -C $(ABSROOTDIR)/ThirdPartyLibraries/LzmaSdk clean
