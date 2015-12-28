
include ../Makefile.conf

-include Makefile.src

INCLUDES = -Iinclude

GC_OBJS = $(addprefix obj/$(PLATFORM_NAME)/gc/,$(notdir $(GC_SRCS:.c=.o)))
DECIMAL_OBJS = $(addprefix obj/$(PLATFORM_NAME)/decimal/,$(notdir $(DECIMAL_SRCS:.c=.o)))
SMILE_OBJS = $(subst src/,obj/$(PLATFORM_NAME)/smile/,$(SMILE_SRCS:.c=.o))

ALL_SRCS = $(GC_SRCS) $(DECIMAL_SRCS) $(SMILE_SRCS)

ALL_OBJS = $(GC_OBJS) $(DECIMAL_OBJS) $(SMILE_OBJS)

GC_DEFS = -DGC_BUILD -DSMILELIB_BUILD
DECIMAL_DEFS = -DSMILELIB_BUILD
SMILE_DEFS = -DGC_BUILD -DSMILELIB_BUILD

SMILELIB = bin/$(PLATFORM_NAME)/libsmile.so

all: $(SMILELIB)
	@echo -e "\nDone."

$(SMILELIB): setup_obj_dirs $(ALL_OBJS)
	$(LINK) $(LINKFLAGS) -shared -o $@ $(ALL_OBJS)

setup_obj_dirs:
	mkdir -p obj/$(PLATFORM_NAME) bin/$(PLATFORM_NAME)

obj/$(PLATFORM_NAME)/gc/%.o : gc/src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) $(GC_DEFS) $(INCLUDES) -Igc/include -c $< -o $@

obj/$(PLATFORM_NAME)/decimal/%.o : decimal/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) $(DECIMAL_DEFS) $(INCLUDES) -c $< -o $@

obj/$(PLATFORM_NAME)/smile/%.o : src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) $(SMILE_DEFS) $(INCLUDES) -c $< -o $@

dep:
	@echo "Generating source list from SmileLib.vcxproj."
	../scripts/makesrc.sh > Makefile.src
	@echo -e "\nGenerating dependencies in Makefile.dep."
	@$(MAKE) dep2
	@echo -e "\nDone."

dep2:
	$(CC) $(CCFLAGS) $(GC_DEFS) $(INCLUDES) -Igc/include -MM $(GC_SRCS) > Makefile.dep
	$(CC) $(CCFLAGS) $(DECIMAL_DEFS) $(INCLUDES) -MM $(DECIMAL_SRCS) >> Makefile.dep
	$(CC) $(CCFLAGS) $(SMILE_DEFS) $(INCLUDES) -MM $(SMILE_SRCS) >> Makefile.dep

clean:
	$(RM) -r obj/$(PLATFORM_NAME) bin/$(PLATFORM_NAME)

distclean: clean
	$(RM) Makefile.dep Makefile.src Makefile.allsrc

