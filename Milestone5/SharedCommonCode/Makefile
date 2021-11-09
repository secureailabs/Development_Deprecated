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

FINDFILES = $(wildcard $(DIR)/*.cpp)
C_FINDFILES = $(wildcard $(DIR)/*.c)

# Add the new include folders here
SHARED_COMMON_INCLUDE :=-I$(shell realpath $(ABSROOTDIR)/SharedCommonCode/Include)

SHARED_COMMON_SRCDIRS :=$(shell realpath $(ABSROOTDIR)/SharedCommonCode/Sources)

SHARED_COMMON_SOURCES := $(foreach DIR,$(SHARED_COMMON_SRCDIRS),$(FINDFILES))
SHARED_COMMON_C_SOURCES := $(foreach DIR,$(SHARED_COMMON_SRCDIRS),$(C_FINDFILES))

SHARED_COMMON_OBJS=$(SHARED_COMMON_SOURCES:$(ABSROOTDIR)%.cpp=$(OBJDIR)%.o)
SHARED_COMMON_OBJS+=$(SHARED_COMMON_C_SOURCES:$(ABSROOTDIR)%.c=$(OBJDIR)%.o)
SHARED_COMMON_LIBRARY=$(ABSROOTDIR)/Binary/StaticLibraries/libSharedCommonCode.a

.PHONY: all, clean

all: $(SHARED_COMMON_LIBRARY)

$(OBJDIR)%.o: $(ABSROOTDIR)%.cpp
	@mkdir -p $(@D)
	$(CXX) $(SHARED_COMMON_CXXFLAGS) $(SHARED_COMMON_INCLUDE) -c $^ -o $@

$(OBJDIR)%.o: $(ABSROOTDIR)%.c
	@mkdir -p $(@D)
	$(CXX) $(SHARED_COMMON_CXXFLAGS) $(SHARED_COMMON_INCLUDE) -c $^ -o $@

$(SHARED_COMMON_LIBRARY): $(SHARED_COMMON_OBJS)
	@mkdir -p $(@D)
	@ar rcs $@ $^

clean:
	@rm $(SHARED_COMMON_LIBRARY) $(SHARED_COMMON_OBJS)
