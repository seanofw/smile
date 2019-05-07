
#-------------------------------------------------------------------------
# Top-level makefile.
#
# This just recursively runs the Makefiles for the individual packages.

include Makefile.conf
include scripts/Makefile.extra

PACKAGES := smilelib smilelibtests smilerunner

SCRIPTS := detect-os.sh detect-proc.sh getsrc.sh makesrc.sh

all dep check clean distclean install install-strip uninstall generated:
	@chmod 755 $(addprefix scripts/,$(SCRIPTS))
	@for package in $(PACKAGES); do $(MAKE) -C $$package $@ || exit 1; done

