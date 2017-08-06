
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

INCLUDES := -Iinclude

DECIMAL_OBJS := $(addprefix obj/$(PLATFORM_NAME)/decimal/,$(notdir $(DECIMAL_SRCS:.c=.o)))
GC_OBJS := $(addprefix obj/$(PLATFORM_NAME)/gc/,$(notdir $(GC_SRCS:.c=.o)))
SMILE_OBJS := $(subst src/,obj/$(PLATFORM_NAME)/smile/,$(SMILE_SRCS:.c=.o))
LIB_OBJS := $(subst lib/,obj/$(PLATFORM_NAME)/lib/,$(LIB_SRCS:.c=.o))

ALL_SRCS := $(DECIMAL_SRCS) $(GC_SRCS) $(LIB_SRCS) $(SMILE_SRCS)

ALL_OBJS := $(DECIMAL_OBJS) $(GC_OBJS) $(LIB_OBJS) $(SMILE_OBJS)

GC_DEFS := -DGC_BUILD -DGC_DLL -DSMILELIB_BUILD
DECIMAL_DEFS := -DSMILELIB_BUILD
SMILE_DEFS := -DGC_BUILD -DSMILELIB_BUILD

SMILELIB := bin/$(PLATFORM_NAME)/libsmile$(SO_EXT)

LIBS := -lm

#---------------------------------------------------------------------------
#  Compile rules.

all: $(SMILELIB)

flarg:
	echo $(LIB_SRCS)
	echo $(LIB_OBJS)

$(SMILELIB): $(ALL_OBJS)
	@$(MKDIR_P) $(dir $@)

	# Use a directory-change trick to shorten the command line before linking.
	cd obj/$(PLATFORM_NAME) ; \
	$(LINKSO) $(ALL_LINKSOFLAGS) -o ../../$@ $(LIBS) $(subst obj/$(PLATFORM_NAME)/,,$(ALL_OBJS)) ; \
	cd ../..

obj/$(PLATFORM_NAME)/decimal/%.o : decimal/%.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(ALL_CFLAGS) $(DECIMAL_DEFS) $(INCLUDES) -c $< -o $@

obj/$(PLATFORM_NAME)/gc/%.o : gc/src/%.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(ALL_CFLAGS) $(GC_DEFS) $(INCLUDES) -Igc/include -c $< -o $@

obj/$(PLATFORM_NAME)/lib/%.o : lib/%.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(ALL_CFLAGS) $(LIB_DEFS) $(INCLUDES) -c $< -o $@

obj/$(PLATFORM_NAME)/smile/%.o : src/%.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(ALL_CFLAGS) $(SMILE_DEFS) $(INCLUDES) -c $< -o $@

#---------------------------------------------------------------------------
#  Source-file and dependency-tracking.  The .vcxproj is treated
#  as the canonical reference for which files belong as part of the
#  library; we use a script to extract them into a make-friendly list
#  in Makefile.src.

dep:
	@$(PRINTF) 'Generating source list from SmileLib.vcxproj.\n'
	$(SHELL) ../scripts/makesrc.sh > Makefile.src
	@$(PRINTF) '\nGenerating dependencies in Makefile.dep (this may take a moment).\n'
	@$(MAKE) dep2

dep2:
	$(RM) Makefile.dep
	$(foreach SRC,$(DECIMAL_SRCS), \
		$(CC) $(ALL_CFLAGS) $(DECIMAL_DEFS) $(INCLUDES) $(DEPFLAGS) $(SRC) -MT 'obj/$(PLATFORM_NAME)/decimal/$(basename $(notdir $(SRC))).o' >> Makefile.dep; $(PRINTF) '\n' >> Makefile.dep;)
	$(foreach SRC,$(GC_SRCS), \
		$(CC) $(ALL_CFLAGS) $(GC_DEFS) $(INCLUDES) -Igc/include $(DEPFLAGS) $(SRC) -MT 'obj/$(PLATFORM_NAME)/gc/$(basename $(notdir $(SRC))).o'  >> Makefile.dep; $(PRINTF) '\n' >> Makefile.dep;)
	$(foreach SRC,$(LIB_SRCS), \
		$(CC) $(ALL_CFLAGS) $(LIB_DEFS) $(INCLUDES) $(DEPFLAGS) $(SRC) -MT 'obj/$(PLATFORM_NAME)/lib/$(basename $(subst lib/,,$(SRC))).o' >> Makefile.dep; $(PRINTF) '\n' >> Makefile.dep;)
	$(foreach SRC,$(SMILE_SRCS), \
		$(CC) $(ALL_CFLAGS) $(SMILE_DEFS) $(INCLUDES) $(DEPFLAGS) $(SRC) -MT 'obj/$(PLATFORM_NAME)/smile/$(basename $(subst src/,,$(SRC))).o' >> Makefile.dep; $(PRINTF) '\n' >> Makefile.dep;)

#---------------------------------------------------------------------------
#  Unit tests (not here).

check: $(SMILELIB)

#---------------------------------------------------------------------------
#  Installation.

install: $(SMILELIB)

install-strip: $(SMILELIB)

uninstall:

#---------------------------------------------------------------------------
#  Cleanup.

clean:
	$(RM_R) obj/$(PLATFORM_NAME) bin/$(PLATFORM_NAME)

distclean: clean
	$(RM) Makefile.dep Makefile.src

#---------------------------------------------------------------------------
#  Dependencies.

-include $(MAKEFILE_DIR)Makefile.dep

