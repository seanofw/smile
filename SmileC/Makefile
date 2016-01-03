
#-------------------------------------------------------------------------
# Top-level makefile.
#
# This just recursively runs the Makefiles for the individual packages.

include Makefile.conf
include scripts/Makefile.extra

PACKAGES = smilelib smilelibtests

all dep check clean distclean install install-strip uninstall:
	@for package in $(PACKAGES); do $(MAKE) -C $$package $@; done

