ABSROOTDIR=$(shell realpath $(ROOTDIR))
OBJDIR=$(ABSROOTDIR)/Binary/Objects

FINDFILES = $(wildcard $(DIR)/*.cpp)
C_FINDFILES = $(wildcard $(DIR)/*.c)

INCLUDE+=-I$(shell realpath $(ABSROOTDIR)/SharedCommonCode/Include)

# We want to find our own static libraries
LDFLAGS += -L$(ABSROOTDIR)/Binary/StaticLibraries

.PHONY: SharedCommonCode

SharedCommonCode:
	@make -C $(ABSROOTDIR)/SharedCommonCode all

