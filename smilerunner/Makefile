
#---------------------------------------------------------------------------
#  Makefile for SmileRunner.
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

LIBS := -lsmile -lm -ledit -lncurses

TESTBIN := bin/$(PLATFORM_NAME)/smile$(BIN_EXT)

LINKRPATH := -Wl,-rpath '-Wl,$$ORIGIN'

#---------------------------------------------------------------------------
#  Compile rules.

all: $(TESTBIN)

$(TESTBIN): $(OBJS) ../smilelib/bin/$(PLATFORM_NAME)/libsmile.so
	@$(MKDIR_P) $(dir $@)

	# Use a directory-change trick to shorten the command line before linking.
	cd obj/$(PLATFORM_NAME) ; \
	$(LINK) $(ALL_LINKFLAGS) $(LINKRPATH) -o ../../$@ $(subst obj/$(PLATFORM_NAME)/,,$(OBJS)) $(addprefix -L../../,$(LIBPATHS)) $(LIBS); \
	cd ../..
	cp $(addsuffix /*,$(LIBPATHS)) bin/$(PLATFORM_NAME)

obj/$(PLATFORM_NAME)/%.o : %.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(ALL_CFLAGS) $(DEFS) $(INCLUDES) -c $< -o $@

#---------------------------------------------------------------------------
#  Source-file and dependency-tracking.  The .vcxproj is treated
#  as the canonical reference for which files belong as part of the
#  library; we use a script to extract them into a make-friendly list
#  in Makefile.src.

dep:
	@$(PRINTF) 'Generating source list from SmileRunner.vcxproj.\n'
	$(SHELL) ../scripts/getsrc.sh SmileRunner.vcxproj > Makefile.src
	@$(PRINTF) '\nGenerating dependencies in Makefile.dep (this may take a moment).\n'
	@$(MAKE) dep2

dep2:
	$(RM) Makefile.dep
	$(foreach SRC,$(SRCS), \
		$(CC) $(ALL_CFLAGS) $(DEFS) $(INCLUDES) $(DEPFLAGS) $(SRC) -MT 'obj/$(PLATFORM_NAME)/$(basename $(SRC)).o' >> Makefile.dep;)

generated:

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

