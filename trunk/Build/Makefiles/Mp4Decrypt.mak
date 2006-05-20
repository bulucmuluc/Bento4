##########################################################################
#
#    Mp4Decrypt Program
#
#    (c) 2001-2002 Gilles Boccon-Gibod
#
##########################################################################
all: mp4decrypt

##########################################################################
# includes
##########################################################################
include $(BUILD_ROOT)/Makefiles/Lib.exp

##########################################################################
# targets
##########################################################################
TARGET_SOURCES = Mp4Decrypt.cpp

##########################################################################
# make path
##########################################################################
VPATH += $(SOURCE_ROOT)/Apps/Mp4Decrypt

##########################################################################
# includes
##########################################################################
include $(BUILD_ROOT)/Makefiles/Rules.mak

##########################################################################
# rules
##########################################################################
mp4decrypt: $(TARGET_OBJECTS) $(TARGET_LIBRARY_FILES)
	$(LINK) $(TARGET_OBJECTS) -o $@ $(LINK_LIBRARIES)


