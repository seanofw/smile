
#---------------------------------------------------------------------------
#  Primary Makefile for SmileLib.
#
#  This is not designed to be user-edited.

#---------------------------------------------------------------------------
#  Includes.

MAKEFILE_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))

include $(MAKEFILE_DIR)../Makefile.conf
include $(MAKEFILE_DIR)../scripts/Makefile.extra

-include $(MAKEFILE_DIR)Makefile.src

#---------------------------------------------------------------------------
#  Definitions.

SMILELIBDIR := ../smilelib

INCLUDES := -I$(SMILELIBDIR)/include

OBJS := $(addprefix obj/$(PLATFORM_NAME)/,$(SRCS:.c=.o))

DEFS := 

LIBPATHS := $(SMILELIBDIR)/bin/$(PLATFORM_NAME)

LIBS := -lsmile -lm

TESTBIN := bin/$(PLATFORM_NAME)/smilelibtests$(BIN_EXT)

#---------------------------------------------------------------------------
#  Compile rules.

all: $(TESTBIN)

$(TESTBIN): $(OBJS)
	@$(MKDIR_P) $(dir $@)

	# Use a directory-change trick to shorten the command line before linking.
	cd obj/$(PLATFORM_NAME) ; \
	$(LINK) $(ALL_LINKFLAGS) -o ../../$@ $(subst obj/$(PLATFORM_NAME)/,,$(OBJS)) $(addprefix -L../../,$(LIBPATHS)) $(LIBS) ; \
	cd ../..

obj/$(PLATFORM_NAME)/%.o : %.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(ALL_CFLAGS) $(DEFS) $(INCLUDES) -c $< -o $@

#---------------------------------------------------------------------------
#  Source-file and dependency-tracking.  The .vcxproj is treated
#  as the canonical reference for which files belong as part of the
#  library; we use a script to extract them into a make-friendly list
#  in Makefile.src.

dep:
	@$(PRINTF) 'Generating source list from SmileLib.vcxproj.\n'
	$(SHELL) ../scripts/getsrc.sh SmileLibTests.vcxproj > Makefile.src
	@$(PRINTF) '\nGenerating dependencies in Makefile.dep (this may take a moment).\n'
	@$(MAKE) dep2

dep2:
	$(RM) Makefile.dep
	$(foreach SRC,$(SRCS), \
		$(CC) $(ALL_CFLAGS) $(DEFS) $(INCLUDES) $(DEPFLAGS) $(SRC) -MT 'obj/$(PLATFORM_NAME)/$(basename $(SRC)).o' >> Makefile.dep;)

#---------------------------------------------------------------------------
#  Unit tests.

check: $(TESTBIN)
	@LD_LIBRARY_PATH=$(LIBPATHS); PATH=$$PATH:$(LIBPATHS); $(TESTBIN)

#---------------------------------------------------------------------------
#  Installation.

install:

install-strip:

uninstall:

#---------------------------------------------------------------------------
#  Cleanup.

clean:
	$(RM_R) obj/$(PLATFORM_NAME) bin/$(PLATFORM_NAME)

distclean: clean
	$(RM) Makefile.dep Makefile.src

