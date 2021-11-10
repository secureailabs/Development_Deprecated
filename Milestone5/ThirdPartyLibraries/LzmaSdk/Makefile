LZMA_CFLAGS=-std=c++17 -fPIC -fstack-protector -DUNIX -DOS_UNIX

BUILD_MODE?=debug
ifeq ($(BUILD_MODE), debug)
	SHARED_COMMON_CXXFLAGS+= -ggdb
else
	SHARED_COMMON_CXXFLAGS+= -O3
endif

ROOTDIR:=../../

ABSROOTDIR=$(shell realpath $(ROOTDIR))
OBJDIR=$(ABSROOTDIR)/Binary/Objects

FINDFILES = $(wildcard $(DIR)/*.c)

# Add the new include folders here
LZMA_INCLUDE :=-I$(shell realpath ./Include/)

LZMA_SRCDIRS :=$(shell realpath ./Sources/)

LZMA_SOURCES := $(foreach DIR,$(LZMA_SRCDIRS),$(FINDFILES))

LZMA_OBJS=$(LZMA_SOURCES:$(ABSROOTDIR)%.c=$(OBJDIR)%.o)
LZMA_LIBRARY=$(ABSROOTDIR)/Binary/StaticLibraries/liblzma-sail.a

.PHONY: all, clean

all: $(LZMA_LIBRARY)

$(OBJDIR)%.o: $(ABSROOTDIR)%.c
	@mkdir -p $(@D)
	$(CXX) $(LZMA_CFLAGS) $(LZMA_INCLUDE) -c $^ -o $@

$(LZMA_LIBRARY): $(LZMA_OBJS)
	@echo $(LZMA_OBJS)
	@mkdir -p $(@D)
	@ar rcs $@ $^

clean:
	@rm $(LZMA_LIBRARY) $(LZMA_OBJS)
