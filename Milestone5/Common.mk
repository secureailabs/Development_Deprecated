ABSROOTDIR=$(shell realpath $(ROOTDIR))
OBJDIR=$(ABSROOTDIR)/Binary/Objects

FINDFILES = $(wildcard $(DIR)/*.cpp)

# Add common include paths once
INCLUDE+=-I$(shell realpath $(ABSROOTDIR)/SharedCommonCode/Include)
INCLUDE+=-I$(shell realpath $(ABSROOTDIR)/ThirdPartyLibraries/LzmaSdk/Include)

# We want to find our own static libraries
LDFLAGS += -L$(ABSROOTDIR)/Binary/StaticLibraries

.PHONY: SharedCommonCode

SharedCommonCode:
	@make -C $(ABSROOTDIR)/SharedCommonCode all

