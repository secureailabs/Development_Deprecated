ABSROOTDIR=$(shell realpath $(ROOTDIR))
OBJDIR=$(ABSROOTDIR)/Binary/Objects

INCLUDE+=-I$(shell realpath $(ABSROOTDIR)/SharedCommonCode/Include)
INCLUDE+=-I$(shell realpath $(ABSROOTDIR)/SharedCommonCode/Include/LzmaSdk)

SRCDIRS+=$(shell realpath $(ABSROOTDIR)/SharedCommonCode/Sources)
SRCDIRS+=$(shell realpath $(ABSROOTDIR)/SharedCommonCode/Sources/LzmaSdk)

FINDFILES = $(wildcard $(DIR)/*.cpp)
C_FINDFILES = $(wildcard $(DIR)/*.c)
