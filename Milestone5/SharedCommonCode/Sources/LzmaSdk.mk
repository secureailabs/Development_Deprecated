
# The Root directory containing all the code
ROOTDIR=../..
ABSROOTDIR=$(shell realpath $(ROOTDIR))
OBJDIR=$(ABSROOTDIR)/Binary/Objects

INCLUDE+=-I$(shell realpath ../../SharedCommonCode/Include/LzmaSdk/)
